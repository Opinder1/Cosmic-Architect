#include "GodotRenderModule.h"
#include "Components.h"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
	GodotRenderModule::GodotRenderModule(flecs::world& world)
	{
        auto* rendering_server = godot::RenderingServer::get_singleton();

		world.module<GodotRenderModule>();

        world.component<Position>();
        world.component<Velocity>();
        world.component<Instance>();
        world.component<Scenario>();
        world.component<Mesh>();

        world.observer<Instance>()
            .event(flecs::OnAdd)
            .iter([rendering_server](flecs::iter& it, Instance* instances)
        {
            for (auto i : it)
            {
                instances[i].instance_id = rendering_server->instance_create();
                godot::UtilityFunctions::print(godot::vformat("created instance %d", instances[i].instance_id.get_id()));
            }
        });

        world.observer<Instance>()
            .event(flecs::OnRemove)
            .iter([rendering_server](flecs::iter& it, Instance* instances)
        {
            for (auto i : it)
            {
                rendering_server->free_rid(instances[i].instance_id);
            }
        });

        /*
        world.system<Position, Velocity>()
            .multi_threaded()
            .iter([](flecs::iter& it, Position* positions, Velocity* velocities)
        {
            for (auto i : it)
            {
                positions[i].position.x += velocities[i].velocity.x * it.delta_time();
                positions[i].position.y += velocities[i].velocity.y * it.delta_time();
            }
        });
        */

        world.observer<Scenario, Instance>()
            .event(flecs::OnSet)
            .iter([rendering_server](flecs::iter& it, Scenario* scenarios, Instance* instances)
        {
            for (auto i : it)
            {
                rendering_server->instance_set_scenario(instances[i].instance_id, scenarios[i].scenario_id);
                godot::UtilityFunctions::print(godot::vformat("set instance %d's scenario to %d", instances[i].instance_id.get_id(), scenarios[i].scenario_id.get_id()));
            }
        });

        /*
        world.system()
            .read<Mesh>(flecs::OnSet)
            .read<Instance>()
            .iter([rendering_server](flecs::iter& it)
        {
            auto meshes = it.field<const Mesh>(1);
            auto instances = it.field<const Instance>(2);
            for (auto i : it)
            {
                rendering_server->instance_set_base(instances[i].instance_id, meshes[i].mesh_id);
            }
        });

        world.system()
            .read<Position>(flecs::OnSet)
            .read<Instance>()
            .iter([rendering_server](flecs::iter& it)
        {
            auto positions = it.field<const Position>(1);
            auto instances = it.field<const Instance>(2);
            for (auto i : it)
            {
                godot::Transform3D transform;
                transform.origin = positions[i].position;
                rendering_server->instance_set_transform(instances[i].instance_id, transform);
            }
        });
        */

        godot::UtilityFunctions::print("Added render module");
	}
}