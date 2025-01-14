#include "LoadingModule.h"
#include "LoadingComponents.h"

#include "Simulation/SimulationComponents.h"

namespace voxel_game::loading
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<sim::Components>();

		world.system<EntityLoader>(DEBUG_ONLY("ProgressEntityLoaders"))
			.each([](EntityLoader& loader)
		{
			loader.Progress();
		});

		world.observer<Saveable, const Identifier, const Database, EntityLoader, const sim::GlobalTime>(DEBUG_ONLY("AutoLoadEntity"))
			.event(flecs::OnAdd)
			.with<AutoLoad>()
			.term_at(2).up()
			.term_at(3).src<EntityLoader>()
			.term_at(4).src<sim::GlobalTime>()
			.each([](flecs::entity entity, Saveable& saveable, const Identifier& identifier, const Database& database, EntityLoader& loader, const sim::GlobalTime& global_time)
		{
			if (saveable.last_load_frame == 0) // Make sure we didn't already load somehow
			{
				loader.LoadEntity(identifier.uuid, entity, database.handle);

				saveable.last_load_frame = global_time.frame_index;
			}
		});

		world.system<Saveable, const AutoSave, const Identifier, const Database, EntityLoader, const sim::GlobalTime>(DEBUG_ONLY("AutoSaveEntites"))
			.immediate() // Do not run staged so that event emmiting is immediately run
			.term_at(2).up()
			.term_at(3).src<EntityLoader>()
			.term_at(4).src<sim::GlobalTime>()
			.each([](flecs::entity entity, Saveable& saveable, const AutoSave& autosave, const Identifier& identifier, const Database& database, EntityLoader& loader, const sim::GlobalTime& global_time)
		{
			if (global_time.frame_index >= saveable.last_save_frame + autosave.frequency) // Wait until we should autosave again
			{
				SaveEvent event;

				entity.world().event<SaveEvent>()
					.ctx(event)
					.entity(entity)
					.emit();

				loader.SaveEntity(identifier.uuid, entity, database.handle, std::move(event.data));

				saveable.last_save_frame = global_time.frame_index;
			}
		});
	}

	void LoadEntity(flecs::world& world, flecs::entity_t entity)
	{
		world.query_builder<Saveable, Identifier, const Database, EntityLoader, sim::GlobalTime>(DEBUG_ONLY("LoadEntity"))
			.term_at(0).src(entity)
			.term_at(1).src(entity)
			.term_at(2).src(entity).up()
			.term_at(3).src<EntityLoader>()
			.term_at(4).src<sim::GlobalTime>()
			.each([](flecs::entity entity, Saveable& saveable, const Identifier& identifier, const Database& database, EntityLoader& loader, const sim::GlobalTime& global_time)
		{
			if (saveable.last_load_frame == 0)
			{
				loader.LoadEntity(identifier.uuid, entity, database.handle);

				saveable.last_load_frame = global_time.frame_index;
			}
		});
	}

	void SaveEntity(flecs::world& world, flecs::entity_t entity)
	{
		world.query_builder<Saveable, const Identifier, const Database, EntityLoader, const sim::GlobalTime>(DEBUG_ONLY("SaveEntity"))
			.term_at(0).src(entity)
			.term_at(1).src(entity)
			.term_at(2).src(entity).up()
			.term_at(3).src<EntityLoader>()
			.term_at(4).src<sim::GlobalTime>()
			.each([](flecs::entity entity, Saveable& saveable, const Identifier& identifier, const Database& database, EntityLoader& loader, const sim::GlobalTime& global_time)
		{
			SaveEvent event;

			entity.world().event<SaveEvent>()
				.ctx(event)
				.entity(entity)
				.emit();

			loader.SaveEntity(identifier.uuid, entity, database.handle, std::move(event.data));

			saveable.last_save_frame = global_time.frame_index;
		});
	}
}