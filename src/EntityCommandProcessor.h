#pragma once

#include "Util/StackAllocator.h"

#include <flecs/flecs.h>

#include <vector>

namespace voxel_game
{
	struct EntityCommandProcessorBase
	{
		size_t state_size;
		void(*state_initialize)(void*, flecs::world_t*, flecs::entity_t);
		void(*state_destroy)(void*);
	};

	template<class ProcessorT>
	struct EntityCommandProcessorExecutor
	{
		static_assert(std::is_base_of_v<EntityCommandProcessorBase, ProcessorT>);

		EntityCommandProcessorExecutor(flecs::entity entity, ProcessorT* processors, size_t num_processors) :
			entity(entity),
			processors(processors),
			num_processors(num_processors)
		{

		}

		template<class CommandIt, class Callable>
		void Run(CommandIt commands_begin, CommandIt commands_end, Callable&& command_callback)
		{
			if (commands_begin == commands_end) // Don't continue if there aren't any commands
			{
				return;
			}

			StackAllocator state_alloc;

			void** states = state_alloc.NewArray<void*>(num_processors);

			for (size_t i = 0; i < num_processors; i++)
			{
				states[i] = state_alloc.Alloc(processors[i].state_size);

				processors[i].state_initialize(states[i], entity.world(), entity);
			}

			auto run_processors_callback = [this, states](auto&&... args)
			{
				for (size_t i = 0; i < num_processors; i++)
				{
					processors[i].process(states[i], args...);
				}
			};

			for (CommandIt commands_it = commands_begin; commands_it != commands_end; commands_it++)
			{
				command_callback(*commands_it, run_processors_callback);
			}

			for (size_t i = 0; i < num_processors; i++)
			{
				processors[i].state_destroy(states[i]);

				state_alloc.Free(states[i]);
			}

			state_alloc.DeleteArray<void*>(states, num_processors);
		}

		flecs::entity entity;
		ProcessorT* processors;
		size_t num_processors;
	};
}