#include "VoxelRenderModule.h"
#include "VoxelRenderComponents.h"

#include "Voxel/VoxelComponents.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include "Render/RenderComponents.h"

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

	void GenerateVertexesForNode(voxel::Context& context, const voxel::Node& node, godot::PackedVector3Array& array, godot::PackedColorArray& voxel_colours)
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
		flecs::world stage;
		const World& render_world;
		const voxel::World& voxel_world;
		spatial3d::World& spatial_world;
		voxel::Context& voxel_ctx;
		rendering::ServerContext& render_ctx;

		void LoadNode(Poly node_poly)
		{
			spatial3d::Node& node = node_poly.GetEntry(spatial_world.node_entry);
			voxel::Node& voxel_node = node_poly.GetEntry(voxel_world.node_entry);
			Node& render_node = node_poly.GetEntry(render_world.node_entry);

			rendering::ThreadContext& thread_context = render_ctx.threads[stage.get_stage_id()];

			render_node.mesh = thread_context.mesh_allocator.RequestRID();
			render_node.mesh_instance = thread_context.instance_allocator.RequestRID();

			thread_context.commands.AddCommand<&godot::RenderingServer::instance_set_base>(render_node.mesh_instance, render_node.mesh);

			godot::PackedVector3Array vertexes;
			godot::PackedColorArray colors;

			// GenerateVertexesForNode(voxel_ctx, voxel_node, vertexes, colors);

			godot::Array arrays;

			arrays.resize(godot::RenderingServer::ARRAY_MAX);

			arrays[godot::RenderingServer::ARRAY_VERTEX] = vertexes;
			arrays[godot::RenderingServer::ARRAY_COLOR] = colors;

			thread_context.commands.AddCommand<&godot::RenderingServer::mesh_add_surface_from_arrays>(render_node.mesh, godot::RenderingServer::PRIMITIVE_TRIANGLES, arrays, godot::Array(), godot::Dictionary(), 0);
			thread_context.commands.AddCommand<&godot::RenderingServer::mesh_surface_set_material>(render_node.mesh, 0, render_world.voxel_material);
		}

		void UnloadNode(Poly node_poly)
		{
			spatial3d::Node& node = node_poly.GetEntry(spatial_world.node_entry);
			voxel::Node& voxel_node = node_poly.GetEntry(voxel_world.node_entry);
			Node& render_node = node_poly.GetEntry(render_world.node_entry);

			rendering::ThreadContext& thread_context = render_ctx.threads[stage.get_stage_id()];

			thread_context.commands.AddCommand<&godot::RenderingServer::free_rid>(render_node.mesh_instance);
			thread_context.commands.AddCommand<&godot::RenderingServer::free_rid>(render_node.mesh);
		}
	};

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<rendering::Components>();
		world.import<voxel::Components>();
		world.import<spatial3d::Module>();

		world.singleton<World>()
			.add_second<spatial3d::WorldMarker>(flecs::With);

		// Initialise the spatial world of a universe
		world.observer<World, spatial3d::WorldMarker, rendering::ServerContext>(DEBUG_ONLY("InitializeVoxelRenderWorld"))
			.event(flecs::OnAdd)
			.term_at(2).singleton().filter()
			.each([](World& voxel_world, spatial3d::WorldMarker& spatial_world, rendering::ServerContext& render_ctx)
		{
			voxel_world.node_entry = spatial_world.world.node_type.AddEntry<Node>();

			voxel_world.voxel_material = render_ctx.main_thread.material_allocator.RequestRID();
		});

		// Initialise the spatial world of a universe
		world.observer<World, rendering::ServerContext>(DEBUG_ONLY("UninitializeVoxelRenderWorld"))
			.event(flecs::OnRemove)
			.term_at(1).singleton().filter()
			.each([](World& voxel_world, rendering::ServerContext& render_ctx)
		{
			render_ctx.main_thread.commands.AddCommand<&godot::RenderingServer::free_rid>(voxel_world.voxel_material);
		});

		world.system<const World, const voxel::World, spatial3d::WorldMarker, voxel::Context, rendering::ServerContext>(DEBUG_ONLY("VoxelRenderNodeModify"))
			.multi_threaded()
			.term_at(3).singleton()
			.term_at(4).singleton()
			.each([](flecs::entity entity, const World& voxel_render_world, const voxel::World& voxel_world, spatial3d::WorldMarker& spatial_world, voxel::Context& voxel_ctx, rendering::ServerContext& render_ctx)
		{
			flecs::world stage = entity.world();

			VoxelRenderNodeLoader loader{ entity, stage, voxel_render_world, voxel_world, spatial_world.world, voxel_ctx, render_ctx };

			for (Poly scale_poly : spatial_world.world.scales)
			{
				spatial3d::Scale& scale = spatial3d::GetScale(spatial_world.world, scale_poly);

				for (Poly node_poly : scale.load_commands)
				{
					loader.LoadNode(node_poly);
				}

				for (Poly node_poly : scale.unload_commands)
				{
					loader.UnloadNode(node_poly);
				}
			}
		});
	}
}