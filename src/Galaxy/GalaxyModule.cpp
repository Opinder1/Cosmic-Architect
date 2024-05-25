#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialProcessors.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	std::unique_ptr<SpatialNode3D> CreateGalaxyNode()
	{
		return std::make_unique<GalaxyNode>();
	}

	void DestroyGalaxyNode(std::unique_ptr<SpatialNode3D>& node)
	{
		std::unique_ptr<GalaxyNode>& galaxy_node = reinterpret_cast<std::unique_ptr<GalaxyNode>&>(node);

		galaxy_node.reset();
	}

	GalaxyModule::GalaxyModule(flecs::world& world)
	{
		world.module<GalaxyModule>("GalaxyModule");

		world.import<SpatialComponents>();
		world.import<GalaxyComponents>();

		world.observer<GalaxyComponent, SpatialWorld3DComponent>()
			.event(flecs::OnAdd)
			.each([](GalaxyComponent& universe, SpatialWorld3DComponent& spatial_world)
		{
			spatial_world.create_node = cb::Bind<&CreateGalaxyNode>();
			spatial_world.destroy_node = cb::Bind<&DestroyGalaxyNode>();
		});
	}
}