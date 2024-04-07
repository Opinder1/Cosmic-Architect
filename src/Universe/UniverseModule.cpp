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
	}

	void UniverseModule::AddComponents(flecs::world& world)
	{
		world.component<ScenarioComponent>();
		world.component<UniverseCameraComponent>();
		world.component<UniverseObjectComponent>();
		world.component<StarRendererComponent>();
		world.component<StarComponent>();
		world.component<GalaxyRendererComponent>();
		world.component<GalaxyComponent>();
		world.component<SimulatedGalaxyComponent>();
	}

	void UniverseModule::AddRelationships(flecs::world& world)
	{
		world.component<StarRendererComponent>().add_second<ScenarioComponent>(flecs::OneOf);
		world.component<GalaxyRendererComponent>().add_second<ScenarioComponent>(flecs::OneOf);
	}
}