#include "GalaxyPrefabs.h"
#include "GalaxyComponents.h"

#include "Render/RenderComponents.h"
#include "Physics3D/PhysicsComponents.h"
#include "Spatial3D/SpatialComponents.h"

#include "Render/RenderModule.h"

#include <flecs/flecs.h>

namespace voxel_game::galaxy
{
	Prefabs::Prefabs(flecs::world& world)
	{
		world.module<Prefabs>();

		world.import<Components>();
		world.import<rendering::Components>();

		world.prefab<PGalaxy>()
			.add<galaxy::CWorld>()
			.add<physics3d::CPosition>()
			.add<physics3d::CScale>();

		world.prefab<PSimulatedGalaxy>()
			.add<physics3d::CPosition>()
			.add<physics3d::CRotation>()
			.add<CWorld>()
			.add<spatial3d::CWorld>()
			.add<spatial3d::CLoader>();

		if (rendering::IsEnabled())
		{
			flecs::entity galaxy_schematic = world.entity();

			galaxy_schematic.add<rendering::CPlaceholderCube>();

			world.entity<PGalaxy>()
				.add<rendering::CTransform>()
				.add<rendering::CInstance>(galaxy_schematic);
		}
	}
}