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
            .each([](const RenderInstance& instance, const RenderScenario& scenario, RenderingServerContext& context)
        {
            RenderingThreadData& thread_data = context.threads[0];

            CommandBuffer::AddCommand(thread_data.commands, "instance_set_scenario", instance.id, scenario.id);
        });

        world.observer<const RenderInstance, const RenderScenario, RenderingServerContext>()
            .event(flecs::OnRemove)
            .term_at(2).parent()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](const RenderInstance& instance, const RenderScenario& scenario, RenderingServerContext& context)
        {
            RenderingThreadData& thread_data = context.threads[0];

            CommandBuffer::AddCommand(thread_data.commands, "instance_set_scenario", instance.id, godot::RID());
        });

        world.observer<const RenderInstance, const RenderMesh, RenderingServerContext>()
            .event(flecs::OnSet)
            .term_at(2).up<RenderBase>()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](const RenderInstance& instance, const RenderMesh& mesh, RenderingServerContext& context)
        {
            RenderingThreadData& thread_data = context.threads[0];

            CommandBuffer::AddCommand(thread_data.commands, "instance_set_base", instance.id, mesh.id);
        });

        world.observer<const RenderInstance, const RenderMesh, RenderingServerContext>()
            .event(flecs::OnRemove)
            .term_at(2).up<RenderBase>()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](const RenderInstance& instance, const RenderMesh& mesh, RenderingServerContext& context)
        {
            RenderingThreadData& thread_data = context.threads[0];

            CommandBuffer::AddCommand(thread_data.commands, "instance_set_base", instance.id, godot::RID());
        });

        world.system<const RenderInstance, RenderingServerContext>(DEBUG_ONLY("UpdateRenderInstanceTransforms"))
            .multi_threaded()
            .term_at(2).src<RenderingServerContext>()
            .term<const Position3DComponent>().optional()
            .term<const Rotation3DComponent>().optional()
            .term<const Scale3DComponent>().optional()
            .each([](flecs::iter& it, size_t, const RenderInstance& instance, RenderingServerContext& context)
        {
            if (!instance.dirty)
            {
                return;
            }

            godot::Transform3D transform;

            if (it.is_set(3))
            {
                auto& position = *it.field<Position3DComponent>(3);

                if (it.is_set(5))
                {
                    auto& scale = *it.field<Scale3DComponent>(4);

                    transform.origin = position.position * scale.scale;
                }
                else
                {
                    transform.origin = position.position;
                }
            }

            if (it.is_set(4))
            {
                auto& rotation = *it.field<Rotation3DComponent>(4);

                transform.rotate(rotation.rotation.get_axis(), rotation.rotation.get_angle());
            }

            RenderingThreadData& thread_data = context.threads[it.world().get_stage_id()];

            CommandBuffer::AddCommand(thread_data.commands, "instance_set_transform", instance.id, transform);
        });
	}
}