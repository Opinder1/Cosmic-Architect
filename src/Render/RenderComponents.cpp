#include "RenderComponents.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
	RenderComponents::RenderComponents(flecs::world& world)
	{
		world.module<RenderComponents>();

        auto* rendering_server = godot::RenderingServer::get_singleton();

        world.component<RenderInstance>()
            .on_add([rendering_server](RenderInstance& instance)
        {
            instance.id = rendering_server->instance_create();
            rendering_server->instance_set_base(instance.id, rendering_server->get_test_cube());
        })
            .on_remove([rendering_server](const RenderInstance& instance)
        {
            rendering_server->free_rid(instance.id);
        });

        world.component<RenderScenario>()
            .on_add([rendering_server](RenderScenario& scenario)
        {
            scenario.id = rendering_server->scenario_create();
        })
            .on_remove([rendering_server](const RenderScenario& scenario)
        {
            rendering_server->free_rid(scenario.id);
        });

        world.component<RenderMesh>()
            .on_add([rendering_server](RenderMesh& mesh)
        {
            mesh.id = rendering_server->mesh_create();
        })
            .on_remove([rendering_server](const RenderMesh& mesh)
        {
            rendering_server->free_rid(mesh.id);
        });
	}
}