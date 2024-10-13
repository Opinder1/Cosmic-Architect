#include "LoadingModule.h"
#include "LoadingComponents.h"

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

	void SaveEntity(flecs::world& world, flecs::entity_t entity)
	{
		SaveEvent event;

		world.event<SaveEvent>()
			.ctx(event)
			.entity(entity)
			.emit();

		world.get_mut<EntityLoader>()->SaveEntity(entity, std::move(event.data));
	}
}