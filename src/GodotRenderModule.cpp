#include "GodotRenderModule.h"
#include "Components.h"

#include <flecs/flecs.h>

namespace voxel_world
{
	void GodotRender(flecs::world& world)
	{
		world.module<int>("GodotRender");

        auto move_sys = world.system<Position, Velocity>();

        move_sys.kind(EcsOnUpdate);
        move_sys.iter([](flecs::iter it, Position* p, Velocity* v)
        {
            for (auto i : it)
            {
                p[i].position.x += v[i].velocity.x * it.delta_time();
                p[i].position.y += v[i].velocity.y * it.delta_time();
            }
        });
	}
}