#include "DrawSystem.h"
#include "Events.h"
#include "Components.h"
#include "Globals.h"

#include "World/Components.h"

#include "Simulation/Simulation.h"
#include "Simulation/Events.h"
#include "Simulation/Components.h"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/rendering_device.hpp>

void DrawSystem::OnInitialize(sim::Simulation& simulation)
{
	simulation.globals.emplace<RenderingGlobal>().server = godot::RenderingServer::get_singleton();

	simulation.dispatcher.Subscribe(cb::BindParam<&DrawSystem::OnProcessNewEntities>(simulation));
	simulation.dispatcher.Subscribe(cb::BindParam<&DrawSystem::OnProcessDeletedEntities>(simulation));
	simulation.dispatcher.Subscribe(cb::BindParam<&DrawSystem::OnSimulationTick>(simulation));
	simulation.dispatcher.Subscribe(cb::BindParam<&DrawSystem::OnRender>(simulation));
}

void DrawSystem::OnShutdown(sim::Simulation& simulation)
{
	simulation.dispatcher.Unsubscribe(cb::BindParam<&DrawSystem::OnRender>(simulation));
	simulation.dispatcher.Unsubscribe(cb::BindParam<&DrawSystem::OnSimulationTick>(simulation));
	simulation.dispatcher.Unsubscribe(cb::BindParam<&DrawSystem::OnProcessDeletedEntities>(simulation));
	simulation.dispatcher.Unsubscribe(cb::BindParam<&DrawSystem::OnProcessNewEntities>(simulation));
}

void DrawSystem::OnProcessNewEntities(sim::Simulation& simulation, const sim::ProcessNewEntitiesEvent& event)
{
	auto& rendering = simulation.globals.get<RenderingGlobal>();

	for (auto&& [entity, rendering_instance] : simulation.registry.view<sim::NewComponent, RenderingInstanceComponent>().each())
	{
		rendering_instance.instance_id = rendering.server->instance_create();

		simulation.registry.emplace<InstanceChangedComponent>(entity);
	}
}

void DrawSystem::OnProcessDeletedEntities(sim::Simulation& simulation, const sim::ProcessDeletedEntitiesEvent& event)
{
	auto& rendering = simulation.globals.get<RenderingGlobal>();

	for (auto&& [entity, rendering_instance] : simulation.registry.view<sim::DeletedComponent, RenderingInstanceComponent>().each())
	{
		rendering.server->free_rid(rendering_instance.instance_id);
	}
}

void DrawSystem::OnSimulationTick(sim::Simulation& simulation, const sim::SimulationTickEvent& event)
{
	auto& rendering = simulation.globals.get<RenderingGlobal>();

	for (auto&& [entity, rendering_instance, transform] : simulation.registry.view<RenderingInstanceComponent, Transform3DComponent, InstanceChangedComponent>().each())
	{
		rendering.server->instance_set_transform(rendering_instance.instance_id, transform.transform);
	}

	for (auto&& [entity, rendering_instance, mesh] : simulation.registry.view<RenderingInstanceComponent, MeshComponent, InstanceChangedComponent>().each())
	{
		rendering.server->instance_set_base(rendering_instance.instance_id, mesh.mesh_id);
	}

	simulation.registry.clear<InstanceChangedComponent>();
}

void DrawSystem::OnRender(sim::Simulation& simulation, const RenderEvent& event)
{
	auto& rendering = simulation.globals.get<RenderingGlobal>();

}

sim::UUID DrawSystem::CreateMeshInstance(sim::Simulation& simulation, godot::RID mesh_id)
{
	entt::entity entity = simulation.registry.create();
	sim::UUID id = simulation.uuid_gen.Generate();

	simulation.registry.emplace<sim::IDComponent>(entity, id);
	simulation.registry.emplace<sim::NewComponent>(entity);
	simulation.registry.emplace<RenderingInstanceComponent>(entity);
	simulation.registry.emplace<Transform3DComponent>(entity);
	simulation.registry.emplace<MeshComponent>(entity);

	return id;
}