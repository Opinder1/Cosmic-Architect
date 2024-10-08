#include "LoadingModule.h"
#include "LoadingComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::loading
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();

		world.system<EntityLoader>()
			.immediate()
			.each([](EntityLoader& loader)
		{
			loader.Progress();
		});
	}

	void SaveEntity(flecs::entity entity)
	{
		entity.world().event<SaveEvent>()
			.ctx(SaveEvent {})
			.entity(entity)
			.enqueue();
	}
}