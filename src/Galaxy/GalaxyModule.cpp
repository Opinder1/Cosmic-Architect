#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Universe/UniverseComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialCommands.h"

#include "Physics/PhysicsComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::galaxy
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<universe::Components>();
		world.import<Components>();
		world.import<spatial::Components>();
		world.import<physics::Components>();


		// Initialise the spatial world of a galaxy
		world.observer<const Galaxy, spatial::World3DComponent>(DEBUG_ONLY("GalaxyInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.each([](const Galaxy& galaxy, spatial::World3DComponent& spatial_world)
		{
			spatial_world.max_scale = spatial::k_max_world_scale;

			spatial_world.builder = spatial::Builder<Scale, Node>();

			for (uint8_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.scales[i] = spatial_world.builder.scale_create();
			}
		});

		// Uninitialize spatial world of a galaxy
		world.observer<const Galaxy, spatial::World3DComponent>(DEBUG_ONLY("GalaxyUninitializeSpatialWorld"))
			.event(flecs::OnRemove)
			.each([](const Galaxy& galaxy, spatial::World3DComponent& spatial_world)
		{
			for (uint8_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.builder.scale_destroy(spatial_world.scales[i]);
			}
		});
	}
}