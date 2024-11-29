#include "RenderComponents.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_game::rendering
{
    Components::Components(flecs::world& world)
	{
		world.module<Components>();

        world.component<TreeNode>();

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