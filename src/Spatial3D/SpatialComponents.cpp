#include "SpatialComponents.h"

namespace voxel_game::spatial3d
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		// Components
		world.component<Entity>();
		world.component<Loader>();
		world.component<RScale>();
		world.component<ScaleMarker>();
		world.component<RegionMarker>();
		world.component<NodeMarker>();
		world.component<WorldMarker>();

		world.component<NodeCreatePhase>();
		world.component<NodeLoadPhase>();
		world.component<NodeUnloadPhase>();
		world.component<NodeDestroyPhase>();

		world.component<WorldCreateEvent>();
		world.component<WorldDestroyEvent>();
	}
}