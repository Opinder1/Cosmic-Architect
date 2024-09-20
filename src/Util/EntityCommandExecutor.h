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
	class EntityCommandExecutor
	{
		static_assert(std::is_base_of_v<EntityCommandProcessorBase, ProcessorT>);

	public:
		EntityCommandExecutor(flecs::entity entity, ProcessorT* processors, size_t num_processors) :
			m_entity(entity),
			m_processors(processors),
			m_num_processors(num_processors)
		{
			m_states = m_state_alloc.NewArray<void*>(m_num_processors);

			for (size_t i = 0; i < num_processors; i++)
			{
				m_states[i] = m_state_alloc.Alloc(m_processors[i].state_size);

				m_processors[i].state_initialize(m_states[i], entity.world(), entity);
			}
		}

		~EntityCommandExecutor()
		{
			for (size_t i = 0; i < m_num_processors; i++)
			{
				m_processors[i].state_destroy(m_states[i]);

				m_state_alloc.Free(m_states[i]);
			}

			m_state_alloc.DeleteArray<void*>(m_states, m_num_processors);
		}

		template<class CommandIt, class Callable>
		void Run(CommandIt commands_begin, CommandIt commands_end, Callable&& command_callback)
		{
			if (commands_begin == commands_end) // Don't continue if there aren't any commands
			{
				return;
			}

			auto run_processors_callback = [this](auto&&... args)
			{
				for (size_t i = 0; i < m_num_processors; i++)
				{
					m_processors[i].process(m_states[i], args...);
				}
			};

			for (CommandIt commands_it = commands_begin; commands_it != commands_end; commands_it++)
			{
				command_callback(*commands_it, run_processors_callback);
			}
		}

	private:
		flecs::entity m_entity;
		ProcessorT* m_processors;
		size_t m_num_processors;

		void** m_states;

		StackAllocator<256> m_state_alloc;
	};
}