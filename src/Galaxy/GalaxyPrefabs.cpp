#include "GalaxyPrefabs.h"
#include "GalaxyComponents.h"

#include "Render/RenderComponents.h"
#include "Render/RenderModule.h"

#include <flecs/flecs.h>

namespace voxel_game::galaxy
{
	Prefabs::Prefabs(flecs::world& world)
	{
		world.module<Prefabs>();

		world.import<Components>();
		world.import<rendering::Components>();

		flecs::entity galaxy_schematic = world.entity();

		galaxy_schematic.add<rendering::PlaceholderCube>();

		world.prefab<GalaxyPrefab>()
			.add<galaxy::World>()
			.add<rendering::UniqueInstance>(galaxy_schematic);
	}
}