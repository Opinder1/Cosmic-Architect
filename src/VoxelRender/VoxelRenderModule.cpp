#include "VoxelRenderModule.h"
#include "VoxelRenderComponents.h"

#include "Voxel/VoxelComponents.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include "Render/RenderComponents.h"
#include "Render/RenderModule.h"

#include "Simulation/SimulationComponents.h"

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>

#include <flecs/flecs.h>

#include "Util/Debug.h"

namespace voxel_game::voxelrender
{
	void AddSquare(godot::PackedVector3Array& array, godot::Vector3 origin, godot::Vector3 right, godot::Vector3 up, bool reverse)
	{
		godot::Vector3 bottom_left = origin;
		godot::Vector3 bottom_right = origin + right;
		godot::Vector3 top_left = origin + up;
		godot::Vector3 top_right = origin + up + right;

		if (reverse)
		{
			array.push_back(bottom_left);
			array.push_back(bottom_right);
			array.push_back(top_right);

			array.push_back(top_right);
			array.push_back(top_left);
			array.push_back(bottom_left);
		}
		else
		{
			array.push_back(bottom_left);
			array.push_back(top_left);
			array.push_back(top_right);

			array.push_back(top_right);
			array.push_back(bottom_right);
			array.push_back(bottom_left);
		}
	}

	void GenerateVertexesForNode(voxel::CContext& context, const voxel::Node& node, godot::PackedVector3Array& array, godot::PackedColorArray& voxel_colours)
	{
		for (size_t x = 0; x < 16; x++)
		for (size_t y = 0; y < 16; y++)
		for (size_t z = 0; z < 16; z++)
		{
			voxel::Voxel voxel = node.voxels[x][y][z];
			const voxel::VoxelType& voxel_type = context.types[voxel.type];
			bool voxel_invisible = voxel_type.cache.is_invisible;

			{
				voxel::Voxel posx = node.voxels[x + 1][y][z];
				const voxel::VoxelType& posx_type = context.types[posx.type];
				bool posx_invisible = posx_type.cache.is_invisible;

				if (voxel_invisible != posx_invisible)
				{
					AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(0, 1, 0), godot::Vector3(0, 0, 1), voxel_invisible > posx_invisible);

					godot::Color color = voxel_invisible > posx_invisible ? voxel_type.cache.color(voxel.data) : posx_type.cache.color(posx.data);

					for (size_t i = 0; i < 6; i++)
					{
						voxel_colours.push_back(color);
					}
				}
			}

			{
				voxel::Voxel posy = node.voxels[x][y + 1][z];
				const voxel::VoxelType& posy_type = context.types[posy.type];
				bool posy_invisible = posy_type.cache.is_invisible;

				if (voxel_invisible != posy_invisible)
				{
					AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(1, 0, 0), godot::Vector3(0, 0, 1), voxel_invisible > posy_invisible);

					godot::Color color = voxel_invisible > posy_invisible ? voxel_type.cache.color(voxel.data) : posy_type.cache.color(posy.data);

					for (size_t i = 0; i < 6; i++)
					{
						voxel_colours.push_back(color);
					}
				}
			}

			{
				voxel::Voxel posz = node.voxels[x][y][z + 1];
				const voxel::VoxelType& posz_type = context.types[posz.type];
				bool posz_invisible = posz_type.cache.is_invisible;

				if (voxel_invisible != posz_invisible)
				{
					AddSquare(array, godot::Vector3(x, y, z), godot::Vector3(1, 0, 0), godot::Vector3(0, 1, 0), voxel_invisible > posz_invisible);

					godot::Color color = voxel_invisible > posz_invisible ? voxel_type.cache.color(voxel.data) : posz_type.cache.color(posz.data);

					for (size_t i = 0; i < 6; i++)
					{
						voxel_colours.push_back(color);
					}
				}
			}
		}
	}

	// Spawns a bunch of flat squares around the camera on the xz plane
	struct VoxelRenderNodeLoader
	{
		flecs::entity entity;
		const spatial3d::Types& types;
		const spatial3d::World& spatial_world;
		voxel::CContext& voxel_ctx;

		void LoadNode(spatial3d::Node* node)
		{
			NODE_TO(node, Node)->mesh = rendering::GetContext().allocator.GetRID(rendering::AllocateType::Mesh);
			NODE_TO(node, Node)->mesh_instance = rendering::GetContext().allocator.GetRID(rendering::AllocateType::Instance);

			ADD_RENDER_CMD(instance_set_base, NODE_TO(node, Node)->mesh_instance, NODE_TO(node, Node)->mesh);

			godot::PackedVector3Array vertexes;
			godot::PackedColorArray colors;

			// GenerateVertexesForNode(voxel_ctx, voxel_node, vertexes, colors);

			godot::Array arrays;

			arrays.resize(godot::RenderingServer::ARRAY_MAX);

			arrays[godot::RenderingServer::ARRAY_VERTEX] = vertexes;
			arrays[godot::RenderingServer::ARRAY_COLOR] = colors;

			ADD_RENDER_CMD(mesh_add_surface_from_arrays, NODE_TO(node, Node)->mesh, godot::RenderingServer::PRIMITIVE_TRIANGLES, arrays, godot::Array(), godot::Dictionary(), 0);
			ADD_RENDER_CMD(mesh_surface_set_material, NODE_TO(node, Node)->mesh, 0, WORLD_TO(spatial_world, World).voxel_material);
		}

		void UnloadNode(spatial3d::Node* node)
		{
			ADD_RENDER_CMD(free_rid, NODE_TO(node, Node)->mesh_instance);
			ADD_RENDER_CMD(free_rid, NODE_TO(node, Node)->mesh);
		}
	};

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<rendering::Components>();
		world.import<voxel::Components>();
		world.import<spatial3d::Module>();

		spatial3d::NodeType::RegisterType<Node>();
		spatial3d::ScaleType::RegisterType<Scale>();
		spatial3d::WorldType::RegisterType<World>();

		// Initialise the spatial world of a universe
		world.observer<spatial3d::CWorld, CWorld>(DEBUG_ONLY("InitializeVoxelRenderWorld"))
			.event(flecs::OnAdd)
			.term_at(2).singleton().filter()
			.each([](spatial3d::CWorld& spatial_world, CWorld& voxel_world)
		{
			spatial_world.types.node_type.AddType<Node>();
			spatial_world.types.scale_type.AddType<Scale>();
			spatial_world.types.world_type.AddType<World>();

			spatial3d::Types& types = spatial_world.types;

			WORLD_TO(spatial_world.world, World)->voxel_material = rendering::GetContext().allocator.GetRID(rendering::AllocateType::Material);
		});

		// Initialise the spatial world of a universe
		world.observer<spatial3d::CWorld, CWorld>(DEBUG_ONLY("UninitializeVoxelRenderWorld"))
			.event(flecs::OnRemove)
			.term_at(1).singleton().filter()
			.each([](spatial3d::CWorld& spatial_world, CWorld& voxel_world)
		{
			spatial3d::Types& types = spatial_world.types;

			ADD_RENDER_CMD(free_rid, WORLD_TO(spatial_world.world, World)->voxel_material);
		});

		world.system<spatial3d::CScale, const spatial3d::CWorld, voxel::CContext>(DEBUG_ONLY("VoxelRenderNodeModify"))
			.multi_threaded()
			.term_at(3).singleton()
			.with<const voxel::CWorld>()
			.with<const CWorld>()
			.each([](flecs::entity entity, spatial3d::CScale& spatial_scale, const spatial3d::CWorld& spatial_world, voxel::CContext& voxel_ctx)
		{
			flecs::world stage = entity.world();

			VoxelRenderNodeLoader loader{ entity, spatial_world.types, *spatial_world.world, voxel_ctx };

			for (spatial3d::Node* node : spatial_scale.scale->load_commands)
			{
				loader.LoadNode(node);
			}

			for (spatial3d::Node* node : spatial_scale.scale->unload_commands)
			{
				loader.UnloadNode(node);
			}
		});
	}
}