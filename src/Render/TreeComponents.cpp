#include "TreeComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::rendering
{
    TreeComponents::TreeComponents(flecs::world& world)
    {
        world.module<TreeComponents>();

        world.component<TreeNode>();
    }
}