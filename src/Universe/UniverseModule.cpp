#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialProcessors.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	std::unique_ptr<SpatialNode3D> CreateUniverseNode()
	{
		return std::make_unique<UniverseNode>();
	}

	void DestroyUniverseNode(std::unique_ptr<SpatialNode3D>& node)
	{
		std::unique_ptr<UniverseNode>& universe_node = reinterpret_cast<std::unique_ptr<UniverseNode>&>(node);

		universe_node.reset();
	}

	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<SpatialComponents>();
		world.import<UniverseComponents>();

		world.observer<UniverseComponent, SpatialWorld3DComponent>()
			.event(flecs::OnAdd)
			.each([](UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
		{
			spatial_world.create_node = cb::Bind<&CreateUniverseNode>();
			spatial_world.destroy_node = cb::Bind<&DestroyUniverseNode>();
		});
	}
}