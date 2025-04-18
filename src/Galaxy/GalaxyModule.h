#pragma once

#include "Entity/EntityPoly.h"

#include "Spatial3D/SpatialPoly.h"

namespace godot
{
	class String;
}

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::galaxy
{
	void Initialize(Simulation& simulation);

	void Uninitialize(Simulation& simulation);

	void Update(Simulation& simulation);

	void WorkerUpdate(Simulation& simulation, size_t index);

	void WorldUpdate(Simulation& simulation, spatial3d::WorldPtr world);

	void ScaleUpdate(Simulation& simulation, spatial3d::ScalePtr scale);

	entity::Ref CreateGalaxy(Simulation& simulation, spatial3d::NodePtr node, godot::Vector3 position, godot::Vector3 scale);

	entity::Ref CreateSimulatedGalaxy(Simulation& simulation, const godot::String& path, entity::WRef universe_entity);

	void DestroySimulatedGalaxy(Simulation& simulation, entity::WRef galaxy);
}