#include "RenderComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::rendering
{
    Components::Components(flecs::world& world)
	{
		world.module<Components>();

        world.component<ServerContext>();
        world.component<Transform>();
        world.component<Scenario>();
        world.component<OwnedScenario>();

        world.component<Instance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive);

        world.component<UniqueInstance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive)
            .add_second<Instance>(flecs::With);

        world.component<Base>();

        world.component<PlaceholderCube>()
            .add_second<Base>(flecs::With);

        world.component<Mesh>()
            .add_second<Base>(flecs::With);
	}
}