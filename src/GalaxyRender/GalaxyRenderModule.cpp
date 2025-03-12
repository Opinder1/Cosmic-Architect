#include "GalaxyRenderModule.h"
#include "GalaxyRenderComponents.h"
#include "GalaxyRenderWorld.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialWorld.h"

namespace voxel_game::galaxyrender
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<spatial3d::Components>();

		spatial3d::NodeType::RegisterType<Node>();
		spatial3d::ScaleType::RegisterType<Scale>();
		spatial3d::WorldType::RegisterType<World>();

		// Initialise the spatial world of a galaxy
		world.observer<spatial3d::CWorld>(DEBUG_ONLY("InitializeGalaxyRenderWorld"))
			.event(flecs::OnAdd)
			.with<const CWorld>()
			.each([](spatial3d::CWorld& spatial_world)
		{
			spatial_world.types.node_type.AddType<Node>();
			spatial_world.types.scale_type.AddType<Scale>();
			spatial_world.types.world_type.AddType<World>();
		});
	}
}