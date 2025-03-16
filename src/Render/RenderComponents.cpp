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

        world.component<CBase>();

        world.component<CInstance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive);

        world.component<CPlaceholderCube>()
            .add_second<CBase>(flecs::With);

        world.component<CMesh>()
            .add_second<CBase>(flecs::With);

		world.component<CPointMesh>()
			.add_second<CBase>(flecs::With);

		world.component<CMultimesh>()
			.add_second<CBase>(flecs::With);

		world.component<CParticles>()
			.add_second<CBase>(flecs::With);

		world.component<CParticlesCollision>()
			.add_second<CBase>(flecs::With);

		world.component<CLight>()
			.add_second<CBase>(flecs::With);

		world.component<CReflectionProbe>()
			.add_second<CBase>(flecs::With);

		world.component<CDecal>()
			.add_second<CBase>(flecs::With);

		world.component<CVoxelGI>()
			.add_second<CBase>(flecs::With);

		world.component<CLightmap>()
			.add_second<CBase>(flecs::With);

		world.component<COccluder>()
			.add_second<CBase>(flecs::With);

		world.component<CFogVolume>()
			.add_second<CBase>(flecs::With);
	}
}