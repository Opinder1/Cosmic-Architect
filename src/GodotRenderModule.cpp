#include "GodotRenderModule.h"
#include "Components.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_world
{
	void GodotRender(flecs::world& world)
	{
        auto* rendering_server = godot::RenderingServer::get_singleton();

		world.module<int>("GodotRender");

        auto move_sys = world.system<Position, Velocity>();

        move_sys.kind(EcsOnUpdate);
        move_sys.iter([](flecs::iter& it, Position* positions, Velocity* velocities)
        {
            for (auto i : it)
            {
                positions[i].position.x += velocities[i].velocity.x * it.delta_time();
                positions[i].position.y += velocities[i].velocity.y * it.delta_time();
            }
        });

        auto add_instance = world.observer<Instance>().event(flecs::OnAdd).read<Mesh>().read<Scenario>();

        add_instance.iter([rendering_server](flecs::iter& it, Instance* instances)
        {
            auto meshes = it.field<Mesh>(1);
            auto scenarios = it.field<Scenario>(2);
            for (auto i : it)
            {
                instances[i].instance_id = rendering_server->instance_create2(meshes[i].mesh_id, scenarios[i].scenario_id);
            }
        });

        auto remove_instance = world.observer<Instance>().event(flecs::OnRemove);

        remove_instance.iter([rendering_server](flecs::iter& it, Instance* instances)
        {
            for (auto i : it)
            {
                rendering_server->free_rid(instances[i].instance_id);
            }
        });

        auto set_scenario = world.observer<Scenario>().event(flecs::OnSet).read<Instance>();

        set_scenario.iter([rendering_server](flecs::iter& it, Scenario* scenarios)
        {
            auto instances = it.field<Instance>(1);
            for (auto i : it)
            {
                rendering_server->instance_set_scenario(instances[i].instance_id, scenarios[i].scenario_id);
            }
        });

        auto set_mesh = world.observer<Mesh>().event(flecs::OnSet).read<Instance>();

        set_mesh.iter([rendering_server](flecs::iter& it, Mesh* meshes)
        {
            auto instances = it.field<Instance>(1);
            for (auto i : it)
            {
                rendering_server->instance_set_base(instances[i].instance_id, meshes[i].mesh_id);
            }
        });

        auto update_instance_pos = world.observer<Position>().event(flecs::OnSet).read<Instance>();
            
        update_instance_pos.iter([rendering_server](flecs::iter& it, Position* positions)
        {
            auto instances = it.field<Instance>(1);
            for (auto i : it)
            {
                godot::Transform3D transform;
                transform.origin = positions[i].position;
                rendering_server->instance_set_transform(instances[i].instance_id, transform);
            }
        });
	}
}