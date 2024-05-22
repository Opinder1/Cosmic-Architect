#include "GalaxyRenderModule.h"
#include "GalaxyComponents.h"

#include "Physics/PhysicsComponents.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
	GalaxyRenderModule::GalaxyRenderModule(flecs::world& world)
	{
		world.module<GalaxyRenderModule>("GalaxyRenderModule");

		world.import<PhysicsComponents>();
		world.import<GalaxyComponents>();

		world.component<GalaxyRenderComponent>();
		world.component<GalaxyRenderContext>();

		auto* rendering_server = godot::RenderingServer::get_singleton();

		world.observer<const GalaxyComponent, const Position3DComponent, const Rotation3DComponent, GalaxyRenderContext>()
			.event(flecs::OnAdd)
			.term_at(2).src<GalaxyRenderContext>()
			.each([rendering_server](flecs::entity entity, const GalaxyComponent& galaxy, const Position3DComponent& position, const Rotation3DComponent rotation, GalaxyRenderContext& render_context)
		{
			GalaxyRenderData data;

			data.position = position.position;
			data.rotation = rotation.rotation;
			data.texture_index = galaxy.galaxy_texture_index;

			std::unique_ptr<GalaxyRenderMesh>& mesh = render_context.node_meshes[position.position / 16];

			// The node doesn't have a mesh yet
			if (!mesh)
			{
				mesh = std::make_unique<GalaxyRenderMesh>();

				mesh->multimesh = rendering_server->multimesh_create();
				rendering_server->multimesh_set_mesh(mesh->multimesh, render_context.mesh);

				mesh->instance = rendering_server->instance_create2(mesh->multimesh, render_context.scenario);
			}

			godot::RID id = mesh->buffer.Add(data);

			rendering_server->multimesh_set_buffer(mesh->multimesh, mesh->buffer.GetBuffer());

			entity.emplace<GalaxyRenderComponent>(id);
		});

		world.observer<const GalaxyComponent, const GalaxyRenderComponent, const Position3DComponent, GalaxyRenderContext>()
			.event(flecs::OnRemove)
			.term_at(2).src<GalaxyRenderContext>()
			.each([rendering_server](const GalaxyComponent& galaxy, const GalaxyRenderComponent galaxy_render, const Position3DComponent& position, GalaxyRenderContext& render_context)
		{
			std::unique_ptr<GalaxyRenderMesh>& mesh = render_context.node_meshes[position.position / 16];

			DEBUG_ASSERT(mesh, "The galaxy should have a mesh its part of");

			mesh->buffer.Remove(galaxy_render.id);

			// No longer any instances using the mesh
			if (mesh->buffer.IsEmpty())
			{
				rendering_server->free_rid(mesh->instance);
				rendering_server->free_rid(mesh->multimesh);

				render_context.node_meshes.erase(position.position / 16);
			}
		});
	}
}