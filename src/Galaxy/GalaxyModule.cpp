#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Universe/UniverseComponents.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialCommands.h"

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
		world.observer<const Galaxy, spatial3d::World>(DEBUG_ONLY("GalaxyInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.each([](const Galaxy& galaxy, spatial3d::World& spatial_world)
		{
			spatial_world.max_scale = spatial3d::k_max_world_scale;

			spatial_world.builder = spatial3d::Builder<Scale, Node>();

			for (uint8_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.scales[i] = spatial_world.builder.scale_create();
			}
		});

		// Uninitialize spatial world of a galaxy
		world.observer<const Galaxy, spatial3d::World>(DEBUG_ONLY("GalaxyUninitializeSpatialWorld"))
			.event(flecs::OnRemove)
			.each([](const Galaxy& galaxy, spatial3d::World& spatial_world)
		{
			for (uint8_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.builder.scale_destroy(spatial_world.scales[i]);
			}
		});
	}
}