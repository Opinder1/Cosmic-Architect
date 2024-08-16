#include "RenderComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	RenderComponents::RenderComponents(flecs::world& world)
	{
		world.module<RenderComponents>();

        world.component<RenderingServerContext>();

        world.component<RenderTreeNode>();

        world.component<RenderScenario>();
        world.component<OwnedRenderScenario>();

        world.component<RenderInstance>();
        world.component<UniqueRenderInstance>();

        world.component<RenderBase>();
        world.component<RenderMesh>();
	}
}