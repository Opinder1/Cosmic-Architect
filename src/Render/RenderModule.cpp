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

        world.observer<const RenderInstance, const RenderMesh, RenderingServerContext>()
            .event(flecs::OnSet)
            .term_at(2).up<RenderBase>()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, const RenderInstance& instance, const RenderMesh& mesh, RenderingServerContext& context)
        {
            auto& commands = context.thread_buffers[entity.world().get_stage_id()];

            CommandBuffer::AddCommand(commands, "instance_set_base", instance.id, mesh.id);
        });

        world.observer<const RenderInstance, const RenderMesh, RenderingServerContext>()
            .event(flecs::OnRemove)
            .term_at(2).up<RenderBase>()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, const RenderInstance& instance, const RenderMesh& mesh, RenderingServerContext& context)
        {
            auto& commands = context.thread_buffers[entity.world().get_stage_id()];

            CommandBuffer::AddCommand(commands, "instance_set_base", instance.id, context.server->get_test_cube());
        });

        world.observer<const RenderInstance, const RenderScenario, RenderingServerContext>()
            .event(flecs::OnSet)
            .term_at(2).parent()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, const RenderInstance& instance, const RenderScenario& scenario, RenderingServerContext& context)
        {
            auto& commands = context.thread_buffers[entity.world().get_stage_id()];

            CommandBuffer::AddCommand(commands, "instance_set_scenario", instance.id, scenario.id);
        });

        world.observer<const RenderInstance, const RenderScenario, RenderingServerContext>()
            .event(flecs::OnRemove)
            .term_at(2).parent()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, const RenderInstance& instance, const RenderScenario& scenario, RenderingServerContext& context)
        {
            auto& commands = context.thread_buffers[entity.world().get_stage_id()];

            CommandBuffer::AddCommand(commands, "instance_set_scenario", instance.id, godot::RID());
        });

        world.system<const RenderInstance, const Position3DComponent, const Rotation3DComponent, RenderingServerContext>()
            .term_at(2).src<RenderingServerContext>().filter()
            .term<const Position3DComponent>().optional()
            .term<const Rotation3DComponent>().optional()
            .each([](flecs::entity entity, const RenderInstance& instance, const Position3DComponent& position, const Rotation3DComponent& rotation, RenderingServerContext& context)
        {
            if (!instance.dirty)
            {
                return;
            }

            auto& commands = context.thread_buffers[entity.world().get_stage_id()];

            godot::Transform3D transform;

            transform.origin = position.position;

            transform.rotate(rotation.rotation.get_axis(), rotation.rotation.get_angle());

            CommandBuffer::AddCommand(commands, "instance_set_transform", instance.id, transform);
        });
	}
}