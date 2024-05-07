#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialProcessors.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	void SpatialLoader3DExample(SpatialWorld3DComponent& spatial_world, const SpatialLoader3DComponent& spatial_loader)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialLoader3DNodeProcessor(spatial_world, spatial_loader, [](...) {});
	}

	void SpatialNode3DExample(SpatialWorld3DComponent& spatial_world, const SpatialNode3DWorkerComponent& spatial_node)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialNode3DNodeProcessor(spatial_world, spatial_node, [](...) {});
	}

	void SpatialRegion3DExample(SpatialWorld3DComponent& spatial_world, const SpatialRegion3DWorkerComponent& spatial_region)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialRegion3DNodeProcessor(spatial_world, spatial_region, [](...) {});
	}

	void SpatialScale3DExample(SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& spatial_scale)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialScale3DNodeProcessor(spatial_world, spatial_scale, [](...) {});
	}

	void SpatialWorld3DExample(SpatialWorld3DComponent& spatial_world)
	{
		SpatialWorld3DNodeProcessor(spatial_world, [](...) {});
	}

	void SpatialCommands3DExample(flecs::entity entity, SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker)
	{
		SpatialScale3DLoadCommandsProcessor(entity.world(), spatial_world, scale_worker, [](...) {});

		SpatialScale3DUnloadCommandsProcessor(entity.world(), spatial_world, scale_worker, [](...) {});
	}

	struct SpatialExampleModule
	{
		SpatialExampleModule(flecs::world& world)
		{
			world.import<SpatialComponents>();

			// Examples

			world.system<SpatialWorld3DComponent, const SpatialLoader3DComponent>("SpatialWorldLoaderExample")
				.multi_threaded()
				.kind<WorldLoaderWorkerPhase>()
				.term_at(1).parent()
				.each(SpatialLoader3DExample);

			world.system<SpatialWorld3DComponent, const SpatialNode3DWorkerComponent>("SpatialWorldNodeExample")
				.multi_threaded()
				.kind<WorldNodeWorkerPhase>()
				.term_at(1).parent()
				.each(SpatialNode3DExample);

			world.system<SpatialWorld3DComponent, const SpatialRegion3DWorkerComponent>("SpatialWorldRegionExample")
				.multi_threaded()
				.kind<WorldRegionWorkerPhase>()
				.term_at(1).parent()
				.each(SpatialRegion3DExample);

			world.system<SpatialWorld3DComponent, const SpatialScale3DWorkerComponent>("SpatialWorldScaleExample")
				.multi_threaded()
				.kind<WorldScaleWorkerPhase>()
				.term_at(1).parent()
				.each(SpatialScale3DExample);

			world.system<SpatialWorld3DComponent>("SpatialWorldExample")
				.multi_threaded()
				.kind<WorldWorkerPhase>()
				.each(SpatialWorld3DExample);

			world.system<SpatialWorld3DComponent, const SpatialScale3DWorkerComponent>("WorldCommandsExample")
				.multi_threaded()
				.kind<WorldWorkerPhase>()
				.term_at(1).parent()
				.each(SpatialCommands3DExample);
		}
	};
}