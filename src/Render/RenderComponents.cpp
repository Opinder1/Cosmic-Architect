#include "RenderComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::rendering
{
    Components::Components(flecs::world& world)
	{
		world.module<Components>();

        world.component<CContext>();
        world.component<CTransform>();
        world.component<CScenario>();
        world.component<COwnedScenario>();

        world.component<CInstance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive);

        world.component<CMultiInstance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive);

        world.component<CBase>();

        world.component<CPlaceholderCube>()
            .add_second<CBase>(flecs::With);

        world.component<CMesh>()
            .add_second<CBase>(flecs::With);
	}
}