#include "UniverseModule.h"
#include "UniverseComponents.h"
#include "GalaxyComponents.h"
#include "Universe.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialProcessors.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	void LoadUniverseNodes(flecs::entity entity, UniverseComponent& universe, SpatialWorld3DComponent& spatial_world, const SpatialScale3DComponent& spatial_scale)
	{
		SpatialScale3DLoaderProcessor(entity.world(), spatial_world, spatial_scale, [&](SpatialCoord3D coord, SpatialNode3D* node)
		{

		});
	}

	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<UniverseComponents>();
		world.import<GalaxyComponents>();
		world.import<SpatialComponents>();

		world.system<UniverseComponent, SpatialWorld3DComponent, const SpatialScale3DComponent>("LoaderKeepAliveNodes")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.term<ParallelWorkerComponent>()
			.term_at(1).parent()
			.term_at(2).parent()
			.each(LoadUniverseNodes);
	}
}