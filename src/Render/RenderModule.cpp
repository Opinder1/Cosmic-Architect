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

        world.observer<const RenderInstance, const RenderScenario, RenderingServerContext>()
            .event(flecs::OnSet)
            .term_at(2).parent()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](flecs::iter it, size_t, const RenderInstance& instance, const RenderScenario& scenario, RenderingServerContext& context)
        {
            CommandBuffer& render_commands = context.thread_buffers[it.world().get_stage_id()];

            CommandBuffer::AddCommand(render_commands, "instance_set_scenario", instance.id, scenario.id);
        });

        world.observer<const RenderInstance, const RenderScenario, RenderingServerContext>()
            .event(flecs::OnRemove)
            .term_at(2).parent()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](flecs::iter it, size_t, const RenderInstance& instance, const RenderScenario& scenario, RenderingServerContext& context)
        {
            CommandBuffer& render_commands = context.thread_buffers[it.world().get_stage_id()];

            CommandBuffer::AddCommand(render_commands, "instance_set_scenario", instance.id, godot::RID());
        });

        world.observer<const RenderInstance, const RenderMesh, RenderingServerContext>()
            .event(flecs::OnSet)
            .term_at(2).up<RenderBase>()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](flecs::iter it, size_t, const RenderInstance& instance, const RenderMesh& mesh, RenderingServerContext& context)
        {
            CommandBuffer& render_commands = context.thread_buffers[it.world().get_stage_id()];

            CommandBuffer::AddCommand(render_commands, "instance_set_base", instance.id, mesh.id);
        });

        world.observer<const RenderInstance, const RenderMesh, RenderingServerContext>()
            .event(flecs::OnRemove)
            .term_at(2).up<RenderBase>()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](flecs::iter it, size_t, const RenderInstance& instance, const RenderMesh& mesh, RenderingServerContext& context)
        {
            CommandBuffer& render_commands = context.thread_buffers[it.world().get_stage_id()];

            CommandBuffer::AddCommand(render_commands, "instance_set_base", instance.id, godot::RID());
        });

        world.observer<RenderInstance, const Position3DComponent>()
            .event(flecs::OnSet)
            .each([](RenderInstance& instance, const Position3DComponent& position)
        {
            instance.dirty = true;
        });

        world.observer<RenderInstance, const Rotation3DComponent>()
            .event(flecs::OnSet)
            .each([](RenderInstance& instance, const Rotation3DComponent& rotation)
        {
            instance.dirty = true;
        });

        world.system<const RenderInstance>()
            .multi_threaded()
            .term<const Position3DComponent>().optional()
            .term<const Rotation3DComponent>().optional()
            .term<RenderingServerContext>().src<RenderingServerContext>()
            .iter([](flecs::iter it, const RenderInstance* instances)
        {
            auto& positions = it.field<const Position3DComponent>(2);
            auto& rotations = it.field<const Rotation3DComponent>(3);

            bool has_positions = it.is_set(2);
            bool has_rotations = it.is_set(3);

            if (!has_positions || !has_rotations)
            {
                return;
            }

            RenderingServerContext& context = *it.field<RenderingServerContext>(4);

            CommandBuffer& render_commands = context.thread_buffers[it.world().get_stage_id()];

            for (size_t index : it)
            {
                if (!instances[index].dirty)
                {
                    return;
                }

                godot::Transform3D transform;

                if (has_positions)
                {
                    transform.origin = positions[index].position;
                }

                if (has_rotations)
                {
                    transform.rotate(rotations[index].rotation.get_axis(), rotations[index].rotation.get_angle());
                }

                CommandBuffer::AddCommand(render_commands, "instance_set_transform", instances[index].id, transform);
            }
        });
	}
}