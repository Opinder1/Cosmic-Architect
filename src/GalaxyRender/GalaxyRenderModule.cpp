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
	}
}