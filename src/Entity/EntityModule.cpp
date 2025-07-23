#include "EntityModule.h"

#include "Components.h"
#include "UniverseSimulation.h"

#include "Simulation/SimulationModule.h"

namespace voxel_game::entity
{
#if defined(DEBUG_ENABLED)
	void SetDebugName(Simulation& simulation, entity::WRef entity, const godot::String& name)
	{
		simulation.entity_factory.AddTypes<CName>(entity.GetID());
		entity->*&CName::name = name;
	}
#endif
}