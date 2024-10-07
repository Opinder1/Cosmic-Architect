#include "LoadingModule.h"
#include "LoadingComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::loading
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();

		// When an entity is set to the loading state this will start the children loading if needed
		world.observer<const Loadable, Loadable>()
			.event(flecs::OnAdd)
			.term_at(0).self()
			.term_at(1).parent().filter() // Filter this so we don't require the child if we alreadt started loading
			.with(LoadState::Unloaded).self()
			.with(LoadState::Loading).parent()
			.each([](flecs::entity entity, const Loadable& loadable, Loadable& parent_loadable)
		{
			if (parent_loadable.require_children)
			{
				parent_loadable.children++;

				entity.add(LoadState::Loading);
			}
		});

		// When an entity is set to the unloading state this will start the children unloading if needed
		world.observer<const Loadable, Loadable>()
			.event(flecs::OnAdd)
			.term_at(0).self()
			.term_at(1).parent().filter() // Filter this so we don't require the child if we already started unloading
			.with(LoadState::Loaded).self()
			.with(LoadState::Unloading).parent()
			.each([](flecs::entity entity, const Loadable& loadable, Loadable& parent_loadable)
		{
			if (parent_loadable.require_children)
			{
				parent_loadable.children++;

				entity.add(LoadState::Unloading);
			}
		});

		// Test entities currently loading/unloading for if they are finished
		world.system<const LoadState, const Loadable, Loadable*>()
			.kind<LoadingPhase>()
			.multi_threaded()
			.term_at(2).parent()
			.each([](flecs::entity entity, const LoadState load_state, const Loadable& loadable, Loadable* parent_loadable)
		{
			if (load_state != LoadState::Loading && load_state != LoadState::Unloading)
			{
				return;
			}

			if (loadable.tasks > 0) // We still have tasks to do so wait
			{
				return;
			}

			if (loadable.require_children && loadable.children > 0) // Our children are still loading so wait
			{
				return;
			}

			if (parent_loadable != nullptr && parent_loadable->require_children)
			{
				parent_loadable->children--;
			}

			if (load_state == LoadState::Loading)
			{
				entity.add(LoadState::Loaded);
			}

			if (load_state == LoadState::Unloading)
			{
				entity.add(LoadState::Unloaded);
			}
		});
	}
}