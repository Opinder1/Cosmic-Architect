#include "TickSystem.h"

#include "Simulation/Simulation.h"
#include "Simulation/Components.h"
#include "Simulation/Events.h"

namespace sim
{
	TickSystem::TickSystem(Simulation& simulation) :
		System(simulation)
	{
		Subscribe<&TickSystem::OnSimulationTick>();
	}

	TickSystem::~TickSystem()
	{

	}

	void TickSystem::OnSimulationTick(const SimulationTickEvent& event)
	{
		Sim().PostEvent(PreTickEvent());
		Sim().PostEvent(TickEvent());
		Sim().PostEvent(PostTickEvent());

		// Process new entities then remove the new entity component flag
		Sim().PostEvent(ProcessNewEntitiesEvent());

		Registry().clear<NewEntityComponent>();

		// Process deleted entities then remove them and their components from the registry
		Sim().PostEvent(ProcessDeletedEntitiesEvent());

		for (auto [entity] : Registry().view<DeletedEntityComponent>().each())
		{
			Registry().destroy(entity);
		}
	}
}