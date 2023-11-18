#include "DrawSystem.h"
#include "Components.h"
#include "Events.h"

#include "World/Components.h"

#include "Simulation/Simulation.h"
#include "Simulation/Events.h"
#include "Simulation/Components.h"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/rendering_device.hpp>

void DrawSystem::OnInitialize(sim::Simulation& simulation)
{
	simulation.Subscribe(cb::BindParam<&DrawSystem::OnProcessNewEntities>(simulation));
	simulation.Subscribe(cb::BindParam<&DrawSystem::OnProcessDeletedEntities>(simulation));
	simulation.Subscribe(cb::BindParam<&DrawSystem::OnSimulationTick>(simulation));
	simulation.Subscribe(cb::BindParam<&DrawSystem::OnRender>(simulation));
}

void DrawSystem::OnShutdown(sim::Simulation& simulation)
{
	simulation.Unsubscribe(cb::BindParam<&DrawSystem::OnRender>(simulation));
	simulation.Unsubscribe(cb::BindParam<&DrawSystem::OnSimulationTick>(simulation));
	simulation.Unsubscribe(cb::BindParam<&DrawSystem::OnProcessDeletedEntities>(simulation));
	simulation.Unsubscribe(cb::BindParam<&DrawSystem::OnProcessNewEntities>(simulation));
}

void DrawSystem::OnProcessNewEntities(sim::Simulation& simulation, const sim::ProcessNewEntitiesEvent& event)
{
	auto* rendering_server = godot::RenderingServer::get_singleton();

	for (auto&& [entity, rendering_instance] : simulation.Registry().view<sim::NewEntityComponent, RenderingInstanceComponent>().each())
	{
		rendering_instance.instance_id = rendering_server->instance_create();

		simulation.Registry().emplace<InstanceChangedComponent>(entity);
	}
}

void DrawSystem::OnProcessDeletedEntities(sim::Simulation& simulation, const sim::ProcessDeletedEntitiesEvent& event)
{
	auto* rendering_server = godot::RenderingServer::get_singleton();

	for (auto&& [entity, rendering_instance] : simulation.Registry().view<sim::DeletedEntityComponent, RenderingInstanceComponent>().each())
	{
		rendering_server->free_rid(rendering_instance.instance_id);
	}
}

void DrawSystem::OnSimulationTick(sim::Simulation& simulation, const sim::SimulationTickEvent& event)
{
	auto* rendering_server = godot::RenderingServer::get_singleton();

	for (auto&& [entity, rendering_instance, transform] : simulation.Registry().view<RenderingInstanceComponent, Transform3DComponent, InstanceChangedComponent>().each())
	{
		rendering_server->instance_set_transform(rendering_instance.instance_id, transform.transform);
	}

	for (auto&& [entity, rendering_instance, mesh] : simulation.Registry().view<RenderingInstanceComponent, MeshComponent, InstanceChangedComponent>().each())
	{
		rendering_server->instance_set_base(rendering_instance.instance_id, mesh.mesh_id);
	}

	simulation.Registry().clear<InstanceChangedComponent>();
}

void DrawSystem::OnRender(sim::Simulation& simulation, const RenderEvent& event)
{
	auto* rendering_server = godot::RenderingServer::get_singleton();

}

sim::UUID DrawSystem::CreateMeshInstance(sim::Simulation& simulation, godot::RID mesh_id)
{
	entt::entity entity = simulation.Registry().create();
	sim::UUID id = simulation.GenerateUUID();

	simulation.Registry().emplace<sim::IDComponent>(entity, id);
	simulation.Registry().emplace<sim::NewEntityComponent>(entity);
	simulation.Registry().emplace<RenderingInstanceComponent>(entity);
	simulation.Registry().emplace<Transform3DComponent>(entity);
	simulation.Registry().emplace<MeshComponent>(entity);

	return id;
}