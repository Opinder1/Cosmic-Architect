#pragma once

#include "Util/VariableLengthArray.h"

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

	template<class ProcessorT, class CommandT, class Callable>
	void RunEntityCommandsWithProcessors(flecs::entity entity, const std::vector<ProcessorT>& processors, const std::vector<CommandT>& commands, Callable&& command_callback)
	{
		static_assert(std::is_base_of_v<EntityCommandProcessorBase, ProcessorT>);

		if (commands.empty()) // Don't continue if there aren't any commands
		{
			return;
		}

		VariableLengthArray<void*> states = MakeVariableLengthArray(void*, processors.size());

		for (size_t i = 0; i < states.size(); i++)
		{
			states[i] = alloca(processors[i].state_size);

			processors[i].state_initialize(states[i], entity.world(), entity);
		}

		auto run_processors_delegate = [&processors, &states](auto&&... args)
		{
			for (size_t i = 0; i < states.size(); i++)
			{
				processors[i].process(states[i], args...);
			}
		};

		for (const CommandT& command : commands)
		{
			command_callback(command, run_processors_delegate);
		}

		for (size_t i = 0; i < states.size(); i++)
		{
			processors[i].state_destroy(states[i]);
		}
	}
}