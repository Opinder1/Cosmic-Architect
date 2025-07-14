#include "EntityModule.h"

#include "EntityComponents.h"

#include "UniverseSimulation.h"

#include "Simulation/SimulationModule.h"

namespace voxel_game::entity
{
#if defined(DEBUG_ENABLED)
	void SetDebugName(Simulation& simulation, entity::WRef entity, const godot::String& name)
	{
		simulation.entity_factory.AddTypes<entity::CName>(entity.GetID());
		entity->*&entity::CName::name = name;
	}
#endif
}