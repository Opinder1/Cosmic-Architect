#pragma once

namespace sim
{
	class Simulation;
	class UUID;

	struct ProcessNewEntitiesEvent;
	struct ProcessDeletedEntitiesEvent;
	struct SimulationTickEvent;
}

namespace godot
{
	class RID;
}

struct RenderEvent;

struct DrawSystem
{
	static void OnInitialize(sim::Simulation& simulation);

	static void OnShutdown(sim::Simulation& simulation);

	static void OnProcessNewEntities(sim::Simulation& simulation, const sim::ProcessNewEntitiesEvent& event);

	static void OnProcessDeletedEntities(sim::Simulation& simulation, const sim::ProcessDeletedEntitiesEvent& event);

	static void OnSimulationTick(sim::Simulation& simulation, const sim::SimulationTickEvent& event);

	static void OnRender(sim::Simulation& simulation, const RenderEvent& event);

	static sim::UUID CreateMeshInstance(sim::Simulation& simulation, godot::RID mesh_id);
};