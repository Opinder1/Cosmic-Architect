#include "EntityModule.h"

#include "EntityComponents.h"

#include "UniverseSimulation.h"

namespace voxel_game::entity
{
	void OnLoadEntity(Simulation& simulation, entity::WRef entity)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		simulation.entity_factory.DoEvent(PolyEvent::BeginLoad, entity);
	}

	void OnUnloadEntity(Simulation& simulation, entity::WRef entity)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation); // Should be called singlethreaded

		simulation.entity_factory.DoEvent(PolyEvent::BeginUnload, entity);
	}

#if defined(DEBUG_ENABLED)
	void SetDebugName(Simulation& simulation, entity::WRef entity, const godot::String& name)
	{
		simulation.entity_factory.AddTypes<entity::CName>(entity.GetID());
		entity->*&entity::CName::name = name;
	}
#endif
}