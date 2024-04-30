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

	void SpatialNode3DExample(SpatialWorld3DComponent& spatial_world, const SpatialNode3DComponent& spatial_node)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialNode3DNodeProcessor(spatial_world, spatial_node, [](...) {});
	}

	void SpatialRegion3DExample(SpatialWorld3DComponent& spatial_world, const SpatialRegion3DComponent& spatial_region)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialRegion3DNodeProcessor(spatial_world, spatial_region, [](...) {});
	}

	void SpatialScale3DExample(SpatialWorld3DComponent& spatial_world, const SpatialScale3DComponent& spatial_scale)
	{
		PARALLEL_ACCESS(spatial_world);

		SpatialScale3DNodeProcessor(spatial_world, spatial_scale, [](...) {});
	}

	void SpatialWorld3DExample(SpatialWorld3DComponent& spatial_world)
	{
		SpatialWorld3DNodeProcessor(spatial_world, [](...) {});
	}

	struct ExampleProcessor
	{
		SpatialNode3D* CreateNode(size_t scale, godot::Vector3i pos) { return new SpatialNode3D(); }

		void DestroyNode(size_t scale, godot::Vector3i pos, SpatialNode3D* node) { delete node; }
	};

	void SpatialCommands3DExample(flecs::entity entity, SpatialWorld3DComponent& spatial_world)
	{
		SpatialCommands3DProcessor(entity, spatial_world, ExampleProcessor());
	}

	struct SpatialExampleModule
	{
		SpatialExampleModule(flecs::world& world)
		{
			world.import<SpatialComponents>();

			// Examples

			world.system<SpatialWorld3DComponent, const SpatialLoader3DComponent>("SpatialWorldLoaderExample")
				.multi_threaded()
				.kind<WorldLoaderProgressPhase>()
				.term<ParallelWorkerComponent>()
				.term_at(1).parent()
				.each(SpatialLoader3DExample);

			world.system<SpatialWorld3DComponent, const SpatialNode3DComponent>("SpatialWorldNodeExample")
				.multi_threaded()
				.kind<WorldNodeProgressPhase>()
				.term<ParallelWorkerComponent>()
				.term_at(1).parent()
				.each(SpatialNode3DExample);

			world.system<SpatialWorld3DComponent, const SpatialRegion3DComponent>("SpatialWorldRegionExample")
				.multi_threaded()
				.kind<WorldRegionProgressPhase>()
				.term<ParallelWorkerComponent>()
				.term_at(1).parent()
				.each(SpatialRegion3DExample);

			world.system<SpatialWorld3DComponent, const SpatialScale3DComponent>("SpatialWorldScaleExample")
				.multi_threaded()
				.kind<WorldScaleProgressPhase>()
				.term<ParallelWorkerComponent>()
				.term_at(1).parent()
				.each(SpatialScale3DExample);

			world.system<SpatialWorld3DComponent>("SpatialWorldExample")
				.multi_threaded()
				.kind<WorldProgressPhase>()
				.each(SpatialWorld3DExample);

			world.system<SpatialWorld3DComponent>("WorldApplyCommands")
				.multi_threaded()
				.kind<WorldProgressPhase>()
				.each(SpatialCommands3DExample);
		}
	};
}