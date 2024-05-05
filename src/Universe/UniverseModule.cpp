#include "UniverseModule.h"
#include "UniverseComponents.h"
#include "GalaxyComponents.h"
#include "Universe.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialProcessors.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	void UniverseLoaderCreateNodes(flecs::entity entity, UniverseComponent& universe, SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker)
	{
		PARALLEL_ACCESS(universe, spatial_world);

		uint8_t scale_index = scale_worker.scale;

		SpatialScale3D& scale = spatial_world.scales[scale_index];

		SpatialScale3DLoaderProcessor(entity.world(), spatial_world, scale_worker, [&scale](SpatialCoord3D coord, SpatialNode3D* node)
		{
			if (node == nullptr)
			{
				scale.nodes.emplace(coord.pos, std::make_unique<UniverseNode>());
			}
		});
	}

	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<UniverseComponents>();
		world.import<GalaxyComponents>();
		world.import<SpatialComponents>();

		world.system<UniverseComponent, SpatialWorld3DComponent, const SpatialScale3DWorkerComponent>("UniverseLoaderCreateNodes")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.term_at(1).parent()
			.term_at(2).parent()
			.each(UniverseLoaderCreateNodes);
	}
}