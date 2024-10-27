#include "SpatialComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::spatial3d
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		// Components
		world.component<Entity>();
		world.component<Loader>();
		world.component<ScaleMarker>();
		world.component<RegionMarker>();
		world.component<World>();

		world.component<NodeCreatePhase>();
		world.component<NodeLoadPhase>();
		world.component<NodeUnloadPhase>();
		world.component<NodeDestroyPhase>();

		world.component<WorldCreateEvent>();
		world.component<WorldDestroyEvent>();
	}
}