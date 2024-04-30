#include "UniverseModule.h"
#include "UniverseComponents.h"
#include "GalaxyComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialProcessors.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	struct UniverseNodeProcessor
	{
		SpatialNode3D* CreateNode(size_t scale, godot::Vector3i pos)
		{
			return new SpatialNode3D();
		}

		void DestroyNode(size_t scale, godot::Vector3i pos, SpatialNode3D* node)
		{
			delete node;
		}
	};

	void UniverseSpatialCommands(flecs::entity entity, UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
	{
		SpatialCommands3DProcessor(entity, spatial_world, UniverseNodeProcessor());
	}

	struct GalaxyNodeProcessor
	{
		SpatialNode3D* CreateNode(size_t scale, godot::Vector3i pos)
		{
			return new SpatialNode3D();
		}

		void DestroyNode(size_t scale, godot::Vector3i pos, SpatialNode3D* node)
		{
			delete node;
		}
	};

	void GalaxySpatialCommands(flecs::entity entity, SimulatedGalaxyComponent& simulated_galaxy, SpatialWorld3DComponent& spatial_world)
	{
		SpatialCommands3DProcessor(entity, spatial_world, GalaxyNodeProcessor());
	}

	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<UniverseComponents>();
		world.import<GalaxyComponents>();
		world.import<SpatialComponents>();

		world.system<UniverseComponent, SpatialWorld3DComponent>("UniverseSpatialCommands")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each(UniverseSpatialCommands);

		world.system<SimulatedGalaxyComponent, SpatialWorld3DComponent>("GalaxySpatialCommands")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each(GalaxySpatialCommands);
	}
}