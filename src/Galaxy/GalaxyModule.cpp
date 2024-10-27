#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Universe/UniverseComponents.h"

#include "Spatial3D/SpatialComponents.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::galaxy
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<universe::Components>();
		world.import<Components>();
		world.import<spatial3d::Components>();
		world.import<physics3d::Components>();

		// Initialise the spatial world of a galaxy
		world.observer<Galaxy, spatial3d::World>(DEBUG_ONLY("GalaxyInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.each([](Galaxy& galaxy, spatial3d::World& spatial_world)
		{
			spatial_world.max_scale = spatial3d::k_max_world_scale;

			galaxy.node_entry = spatial_world.node_type.AddEntry<Node>();
		});
	}
}