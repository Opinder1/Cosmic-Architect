#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialProcessors.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	void UniverseLoaderCreateNodes(flecs::entity entity, UniverseComponent& universe, SpatialWorld3DComponent& spatial_world, const SpatialScale3DWorkerComponent& scale_worker, SpatialLoadCommands3DComponent& load_commands)
	{
		PARALLEL_ACCESS(universe, spatial_world);

		uint8_t scale_index = scale_worker.scale;

		SpatialScale3D& scale = spatial_world.scales[scale_index];

		std::vector<godot::Vector3i>& commands = load_commands.scales[scale_index];

		SpatialScale3DLoaderProcessor(entity.world(), spatial_world, scale_worker, [&scale, &commands](SpatialCoord3D coord, SpatialNode3D* node)
		{
			if (node == nullptr)
			{
				scale.nodes.emplace(coord.pos, std::make_unique<UniverseNode>());
				commands.push_back(coord.pos);
			}
		});
	}

	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<SpatialComponents>();
		world.import<UniverseComponents>();

		world.system<UniverseComponent, SpatialWorld3DComponent, const SpatialScale3DWorkerComponent, SpatialLoadCommands3DComponent>("UniverseLoaderCreateNodes")
			.multi_threaded()
			.kind<WorldScaleWorkerPhase>()
			.term_at(1).parent()
			.term_at(2).parent()
			.interval(1.0 / 20.0)
			.each(UniverseLoaderCreateNodes);
	}
}