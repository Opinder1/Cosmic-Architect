#include "GalaxyRenderModule.h"
#include "GalaxyComponents.h"

#include "Physics/PhysicsComponents.h"

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

			if (!mesh)
			{
				mesh = std::make_unique<GalaxyRenderMesh>();
			}

			godot::RID id = mesh->buffer.Add(data);

			rendering_server->multimesh_set_buffer(mesh->mesh, mesh->buffer.GetBuffer());

			entity.emplace<GalaxyRenderComponent>(id);
		});
	}
}