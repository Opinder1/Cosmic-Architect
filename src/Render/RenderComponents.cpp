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

        world.component<CInstance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive);

        world.component<CBase>();

        world.component<CPlaceholderCube>()
            .add_second<CBase>(flecs::With);

        world.component<CMesh>()
            .add_second<CBase>(flecs::With);

		world.component<CPointMesh>();
		world.component<CMultimesh>();
		world.component<CParticles>();
		world.component<CParticlesCollision>();
		world.component<CLight>();
		world.component<CReflectionProbe>();
		world.component<CDecal>();
		world.component<CVoxelGI>();
		world.component<CLightmap>();
		world.component<COccluder>();
		world.component<CFogVolume>();
	}
}