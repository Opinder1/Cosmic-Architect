#include "SpatialComponents.h"

namespace voxel_game::spatial3d
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		// Components
		world.component<CLoader>();
		world.component<CRegion>();
		world.component<CEntity>();
		world.component<CScale>();
		world.component<CWorld>();

		world.component<PNodeCreate>();
		world.component<PNodeLoad>();
		world.component<PNodeUnload>();
		world.component<PNodeDestroy>();

		world.component<EWorldCreate>();
		world.component<EWorldDestroy>();

		world.component<CWorld>()
			.on_remove([](CWorld& world)
		{
			world.types.world_type.DestroyPoly(world.world);
		});
	}
}