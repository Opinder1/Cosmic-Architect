#include "UniverseModule.h"
#include "UniverseComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		AddComponents(world);
		AddRelationships(world);
		AddObservers(world);
		AddSystems(world);
	}

	void UniverseModule::AddComponents(flecs::world& world)
	{
		world.component<ScenarioComponent>();
		world.component<UniverseCameraComponent>();
		world.component<UniverseObjectComponent>();
		world.component<StarComponent>();
		world.component<GalaxyComponent>();
		world.component<SimulatedGalaxyComponent>();
	}

	void UniverseModule::AddRelationships(flecs::world& world)
	{

	}

	void UniverseModule::AddObservers(flecs::world& world)
	{

	}

	void UniverseModule::AddSystems(flecs::world& world)
	{

	}
}