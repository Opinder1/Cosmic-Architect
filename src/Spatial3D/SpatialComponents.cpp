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

		world.component<CLoader>()
			.on_add([](CLoader& loader)
		{
			loader.loader = new Loader();
		})
			.on_remove([](CLoader& loader)
		{
			delete loader.loader;
		});

		world.component<CWorld>()
			.on_remove([](CWorld& world)
		{
			Types* types = world.world.poly->types;

			types->world_type.DestroyPoly(world.world.poly);
		});
	}
}