#include "RenderComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	RenderComponents::RenderComponents(flecs::world& world)
	{
		world.module<RenderComponents>();

        world.component<RenderingServerContext>();
        world.component<RenderScenario>();
        world.component<OwnedScenario>();
        world.component<RenderInstance>();
        world.component<RenderInstanceFlags>();
        world.component<RenderBase>();
        world.component<RenderMesh>();
        world.component<RenderMultiInstance>();
        world.component<RenderMultiMesh>();
        world.component<FlatTextureComponent>();
	}
}