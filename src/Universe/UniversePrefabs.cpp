#include "UniversePrefabs.h"
#include "UniverseComponents.h"

#include "Render/RenderComponents.h"
#include "Simulation/SimulationComponents.h"
#include "Spatial3D/SpatialComponents.h"

#include "Render/RenderModule.h"

#include <flecs/flecs.h>

namespace voxel_game::universe
{
	Prefabs::Prefabs(flecs::world& world)
	{
		world.module<Prefabs>();

		world.import<Components>();
		world.import<spatial3d::Components>();
		world.import<sim::Components>();

		world.prefab<PSimulatedUniverse>()
			.add<universe::CWorld>()
			.add<universe::CWorld>()
			.add<spatial3d::CWorld>()
			.add<sim::CConfig>();

		if (rendering::IsEnabled())
		{
			world.entity<PSimulatedUniverse>()
				.add<rendering::CTransform>();
		}
	}
}