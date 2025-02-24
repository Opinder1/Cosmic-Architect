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

		galaxy_schematic.add<rendering::CPlaceholderCube>();

		world.prefab<GalaxyPrefab>()
			.add<galaxy::CWorld>()
			.add<rendering::CInstance>(galaxy_schematic);
	}
}