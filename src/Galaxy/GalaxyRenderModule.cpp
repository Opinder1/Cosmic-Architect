#include "GalaxyRenderModule.h"
#include "GalaxyComponents.h"

#include "Physics/PhysicsComponents.h"

#include "Util/VariableLengthArray.h"
#include "Util/Debug.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
	void RebuildBuffer(GalaxyRenderMesh& mesh, godot::RenderingServer& rendering_server)
	{
		VariableLengthArray<godot::RID> rids = MakeVariableLengthArray(godot::RID, mesh.render_data.get_rid_count());
		mesh.render_data.fill_owned_buffer(rids.data());

#if 1
		for (uint32_t i = 0; i < rids.size(); i++)
		{
			GalaxyRenderData* render_data = mesh.render_data.get_or_null(rids[i]);

			godot::Transform3D transform;
			transform.translate_local(render_data->position);
			rendering_server.multimesh_instance_set_transform(mesh.multimesh, i, transform);
		}
#else
		godot::PackedFloat32Array buffer;
		buffer.resize(rids.size() * sizeof(godot::Transform3D));

		godot::Transform3D* data = reinterpret_cast<godot::Transform3D*>(buffer.ptrw());

		for (uint32_t i = 0; i < rids.size(); i++)
		{
			GalaxyRenderData* render_data = mesh.render_data.get_or_null(rids[i]);

			data[i] = godot::Transform3D();
			data[i].translate_local(render_data->position);
			data[i].rotate(render_data->rotation.get_axis(), render_data->rotation.get_angle());
		}

		rendering_server.multimesh_set_buffer(mesh.multimesh, buffer);
#endif
	}

	GalaxyRenderModule::GalaxyRenderModule(flecs::world& world)
	{
		world.module<GalaxyRenderModule>("GalaxyRenderModule");

		world.import<PhysicsComponents>();
		world.import<GalaxyComponents>();

		world.component<GalaxyRenderComponent>();
		world.component<GalaxyRenderContext>();

		auto& rendering_server = *godot::RenderingServer::get_singleton();

		world.observer<const GalaxyComponent, const Position3DComponent, const Rotation3DComponent, GalaxyRenderContext>()
			.event(flecs::OnAdd)
			.yield_existing()
			.term_at(4).src<GalaxyRenderContext>()
			.each([&rendering_server](flecs::entity entity, const GalaxyComponent& galaxy, const Position3DComponent& position, const Rotation3DComponent& rotation, GalaxyRenderContext& render_context)
		{
			godot::Vector3i mesh_pos = position.position / 16;

			std::unique_ptr<GalaxyRenderMesh>& mesh = render_context.node_meshes[mesh_pos];

			// The node doesn't have a mesh yet
			if (!mesh)
			{
				mesh = std::make_unique<GalaxyRenderMesh>();

				mesh->multimesh = rendering_server.multimesh_create();
				rendering_server.multimesh_set_mesh(mesh->multimesh, render_context.mesh);

				mesh->instance = rendering_server.instance_create2(mesh->multimesh, render_context.scenario);
			}

			GalaxyRenderData render_data;

			render_data.position = position.position;
			render_data.rotation = rotation.rotation;
			render_data.texture_index = galaxy.galaxy_texture_index;

			godot::RID id = mesh->render_data.make_rid(render_data);

			entity.emplace<GalaxyRenderComponent>(id, mesh_pos);

			mesh->dirty = true;
		});

		world.observer<GalaxyRenderComponent, const GalaxyComponent, const Position3DComponent, const Rotation3DComponent, GalaxyRenderContext>()
			.event(flecs::OnSet)
			.term_at(5).src<GalaxyRenderContext>()
			.each([&rendering_server](GalaxyRenderComponent& galaxy_render, const GalaxyComponent& galaxy, const Position3DComponent& position, const Rotation3DComponent& rotation, GalaxyRenderContext& render_context)
		{
			godot::Vector3i mesh_pos = galaxy_render.mesh_pos;

			godot::Vector3i new_mesh_pos = position.position / 16;

			if (new_mesh_pos != mesh_pos)
			{
				std::unique_ptr<GalaxyRenderMesh>& old_mesh = render_context.node_meshes[mesh_pos];

				old_mesh->render_data.free(galaxy_render.id);

				old_mesh->dirty = true;

				std::unique_ptr<GalaxyRenderMesh>& new_mesh = render_context.node_meshes[mesh_pos];

				GalaxyRenderData render_data;

				render_data.position = position.position;
				render_data.rotation = rotation.rotation;
				render_data.texture_index = galaxy.galaxy_texture_index;

				godot::RID id = new_mesh->render_data.make_rid(render_data);

				galaxy_render.id = id;

				new_mesh->dirty = true;
			}
			else
			{
				std::unique_ptr<GalaxyRenderMesh>& mesh = render_context.node_meshes[mesh_pos];

				GalaxyRenderData* render_data = mesh->render_data.get_or_null(galaxy_render.id);

				render_data->position = position.position;
				render_data->rotation = rotation.rotation;
				render_data->texture_index = galaxy.galaxy_texture_index;

				mesh->dirty = true;
			}
		});

		world.observer<const GalaxyRenderComponent, const GalaxyComponent, GalaxyRenderContext>()
			.event(flecs::OnRemove)
			.term_at(3).src<GalaxyRenderContext>()
			.each([&rendering_server](const GalaxyRenderComponent& galaxy_render, const GalaxyComponent& galaxy, GalaxyRenderContext& render_context)
		{
			godot::Vector3i mesh_pos = galaxy_render.mesh_pos;

			std::unique_ptr<GalaxyRenderMesh>& mesh = render_context.node_meshes[mesh_pos];

			DEBUG_ASSERT(mesh, "The galaxy should have a mesh its part of");

			mesh->render_data.free(galaxy_render.id);

			// No longer any instances using the mesh
			if (mesh->render_data.get_rid_count() == 0)
			{
				rendering_server.free_rid(mesh->instance);
				rendering_server.free_rid(mesh->multimesh);

				render_context.node_meshes.erase(mesh_pos);
			}

			mesh->dirty = true;
		});

		world.system<GalaxyRenderContext>()
			.each([&rendering_server](GalaxyRenderContext& render_context)
		{
			for (auto& [pos, mesh] : render_context.node_meshes)
			{
				DEBUG_ASSERT(mesh, "The mesh should be valid");

				if (!mesh->dirty)
				{
					continue;
				}

				if (mesh->allocated < mesh->render_data.get_rid_count());
				{
					mesh->allocated = mesh->render_data.get_rid_count();
					rendering_server.multimesh_allocate_data(mesh->multimesh, mesh->allocated, godot::RenderingServer::MULTIMESH_TRANSFORM_3D, false, false);
				}

				RebuildBuffer(*mesh, rendering_server);

				rendering_server.multimesh_set_visible_instances(mesh->multimesh, mesh->render_data.get_rid_count());

				mesh->dirty = false;
			}
		});
	}
}