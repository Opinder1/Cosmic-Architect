#include "SimulationComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	SimulationComponents::SimulationComponents(flecs::world& world)
	{
		world.module<SimulationComponents>();
		
		world.component<SimulationTime>();
		world.component<LocalTime>();
	}
}