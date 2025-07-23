#include "VoxelRenderModule.h"
#include "VoxelRenderWorld.h"

#include "Spatial3D/SpatialModule.h"
#include "Render/RenderModule.h"

#include "Components.h"

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>

#include "Util/Debug.h"

namespace voxel_game::voxelrender
{
	class VoxelRenderer
	{
	public:
		VoxelRenderer()
		{
			m_compositor = RS::get_singleton()->compositor_create();
			m_compositor_effect = RS::get_singleton()->compositor_effect_create();

			RS::get_singleton()->compositor_effect_set_flag(m_compositor_effect, RS::COMPOSITOR_EFFECT_FLAG_ACCESS_RESOLVED_DEPTH, true);

			godot::TypedArray<godot::RID> effects;
			effects.push_back(m_compositor_effect);
			RS::get_singleton()->compositor_set_compositor_effects(m_compositor, effects);
		}

	private:
		godot::RID m_compositor;
		godot::RID m_compositor_effect;
	};

	/*
	// Spawns a bunch of flat squares around the camera on the xz plane
	struct VoxelRenderNodeLoader
	{
		flecs::entity entity;
		spatial3d::WorldPtr spatial_world;
		voxel::CContext& voxel_ctx;
		CContext& render_ctx;

		void LoadNode(spatial3d::NodePtr node)
		{
			node->*&Node::mesh = rendering::AllocRID(rendering::RIDType::Mesh);
			node->*&Node::mesh_instance = rendering::AllocRID(rendering::RIDType::Instance);

			rendering::AddCommand<&RS::instance_set_base>(node->*&Node::mesh_instance, node->*&Node::mesh);

			godot::PackedVector3Array vertexes;
			godot::PackedColorArray colors;

			// GenerateVertexesForNode(voxel_ctx, voxel_node, vertexes, colors);

			godot::Array arrays;

			arrays.resize(godot::RenderingServer::ARRAY_MAX);

			arrays[godot::RenderingServer::ARRAY_VERTEX] = vertexes;
			arrays[godot::RenderingServer::ARRAY_COLOR] = colors;

			rendering::AddCommand<&RS::mesh_add_surface_from_arrays>(node->*&Node::mesh, godot::RenderingServer::PRIMITIVE_TRIANGLES, arrays, godot::Array(), godot::Dictionary(), 0);
			rendering::AddCommand<&RS::mesh_surface_set_material>(node->*&Node::mesh, 0, spatial_world->*&voxelrender::World::voxel_material);
		}

		void UnloadNode(spatial3d::NodePtr node)
		{
			rendering::AddCommand<&RS::free_rid>(node->*&Node::mesh_instance);
			rendering::AddCommand<&RS::free_rid>(node->*&Node::mesh);
		}
	};

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<rendering::Components>();
		world.import<voxel::Components>();
		world.import<spatial3d::Module>();

		// Initialise the spatial world of a universe
		world.observer<spatial3d::CWorld>(DEBUG_ONLY("UninitializeVoxelRenderWorld"))
			.event(flecs::OnRemove)
			.with<const CWorld>()
			.each([](spatial3d::CWorld& spatial_world)
		{
			godot::RID voxel_material = spatial_world.world->*&World::voxel_material;

			if (voxel_material.is_valid())
			{
				rendering::AddCommand<&RS::free_rid>(voxel_material);
			}
		});

		world.system<spatial3d::CScale, const spatial3d::CWorld, voxel::CContext, CContext>(DEBUG_ONLY("VoxelRenderNodeModify"))
			.multi_threaded()
			.term_at(1).up(flecs::ChildOf)
			.term_at(2).singleton()
			.term_at(3).singleton()
			.with<const voxel::CWorld>().up(flecs::ChildOf)
			.with<const CWorld>().up(flecs::ChildOf)
			.each([](flecs::entity entity, spatial3d::CScale& spatial_scale, const spatial3d::CWorld& spatial_world, voxel::CContext& voxel_ctx, CContext& render_ctx)
		{
			VoxelRenderNodeLoader loader{ entity, spatial_world.world, voxel_ctx, render_ctx };

			for (spatial3d::NodePtr node : spatial_scale.scale->*&spatial3d::Scale::load_commands)
			{
				loader.LoadNode(node);
			}

			for (spatial3d::NodePtr node : spatial_scale.scale->*&spatial3d::Scale::unload_commands)
			{
				loader.UnloadNode(node);
			}
		});
	}
	*/
}