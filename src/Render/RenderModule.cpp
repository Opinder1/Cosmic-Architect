#include "RenderModule.h"
#include "RenderComponents.h"

#include "Physics/PhysicsComponents.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
	RenderModule::RenderModule(flecs::world& world)
	{
		world.module<RenderModule>();

		world.import<RenderComponents>();

        auto* rendering_server = godot::RenderingServer::get_singleton();

        world.observer<const RenderInstance, const RenderMesh>()
            .event(flecs::OnSet)
            .term_at(2).up<RenderBase>()
            .each([rendering_server](const RenderInstance& instance, const RenderMesh& mesh)
        {
            rendering_server->instance_set_base(instance.id, mesh.id);
        });

        world.observer<const RenderInstance, const RenderMesh>()
            .event(flecs::OnRemove)
            .term_at(2).up<RenderBase>()
            .each([rendering_server](const RenderInstance& instance, const RenderMesh& mesh)
        {
            rendering_server->instance_set_base(instance.id, rendering_server->get_test_cube());
        });

        world.observer<const RenderInstance, const RenderScenario>()
            .event(flecs::OnSet)
            .term_at(2).parent()
            .each([rendering_server](const RenderInstance& instance, const RenderScenario& scenario)
        {
            rendering_server->instance_set_scenario(instance.id, scenario.id);
        });

        world.observer<const RenderInstance, const RenderScenario>()
            .event(flecs::OnRemove)
            .term_at(2).parent()
            .each([rendering_server](const RenderInstance& instance, const RenderScenario& scenario)
        {
            rendering_server->instance_set_scenario(instance.id, godot::RID());
        });

        world.system<const RenderInstance, const Position3DComponent>()
            .each([rendering_server](const RenderInstance& instance, const Position3DComponent& position)
        {
            godot::Transform3D transform;
            transform.origin = position.position;
            rendering_server->instance_set_transform(instance.id, transform);
        });
	}
}