#pragma once

#include "CommandBufferEntry.h"

#include "Util/Util.h"
#include "Util/Nocopy.h"
#include "Util/Debug.h"

#include <utility>
#include <tuple>

namespace voxel_game
{
	using TCommand = std::byte* (*)(void*, std::byte*, std::byte*, bool);

	class TCommandBufferBase : Nocopy
	{
		template<class T>
		friend class TCommandBufferEntry;

	public:
		TCommandBufferBase();

		~TCommandBufferBase();

		TCommandBufferBase(TCommandBufferBase&& other) noexcept;

		TCommandBufferBase& operator=(TCommandBufferBase&& other) noexcept;

		size_t NumCommands() const;

		// Destroy all remaining commands and arguments
		void Clear();

		void ShrinkToFit();

	protected:
		size_t ProcessCommandsUntyped(void* object, size_t max);

	protected:
		Storage m_data;
		size_t m_start = 0;
		size_t m_num_commands = 0;
	};

	template<class T>
	class TCommandBuffer : public TCommandBufferBase
	{
	public:
		// Register a new command for the queue
		template<auto Method, class... Args>
		void AddCommand(Args&&... args)
		{
			DEBUG_ASSERT(m_start == 0, "We shouldn't be adding commands when we are already processing the buffer");

			using Derived = typename get_method_class<decltype(Method)>::type;

			static_assert(std::is_base_of_v<T, Derived> || std::is_base_of_v<Derived, T>);

			DefaultCommand<Method, T, Derived, Args...>::Write(m_data, std::forward<Args>(args)...);

			m_num_commands++;
		}

		// Process only up to a certain number of commands and return how many were processed (0 for max to process all)
		size_t ProcessCommands(T& object, size_t max = k_process_all_commands)
		{
			return ProcessCommandsUntyped(&object, max);
		}
	};

	// Write a type to a buffer
	template<class T>
	std::byte* ReadType(std::byte* buffer_pos, std::byte* buffer_end, T*& value)
	{
		DEBUG_ASSERT(buffer_pos + sizeof(T) <= buffer_end, "Not enough space to read another encoded variant value");

		value = reinterpret_cast<T*>(buffer_pos);

		return buffer_pos + sizeof(T);
	}

	// Write a type to a buffer
	template<class T>
	std::byte* DestroyType(std::byte* buffer_pos, std::byte* buffer_end)
	{
		DEBUG_ASSERT(buffer_pos + sizeof(T) <= buffer_end, "Not enough space to read another encoded variant value");

		T* value = reinterpret_cast<T*>(buffer_pos);

		std::destroy_at(value);

		return buffer_pos + sizeof(T);
	}

	// A templated method wrapper that reads all arguments needed to call the method.
	template<auto Method, class Base, class Derived, class... Args>
	struct DefaultCommand
	{
		// Storage type for arguments being stored in memory. A tuple is used to make sure alignment of members is correct
		using ArgStorage = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

		template<size_t... Indexes>
		static void Call(Base& object, ArgStorage& args, std::index_sequence<Indexes...>)
		{
			(static_cast<Derived&>(object).*Method)(std::forward<Args>(std::get<Indexes>(args))...);
		}

		static std::byte* Read(void* object, std::byte* buffer_pos, std::byte* buffer_end, bool execute)
		{
			if (execute)
			{
				ArgStorage* args;
				ReadType<ArgStorage>(buffer_pos, buffer_end, args);

				Call(*static_cast<Base*>(object), *args, std::index_sequence_for<Args...>{});
			}

			return DestroyType<ArgStorage>(buffer_pos, buffer_end);
		}

		static void Write(Storage& storage, Args&&... args)
		{
			WriteType<TCommand>(storage, &DefaultCommand::Read);

			WriteType<ArgStorage>(storage, ArgStorage(std::forward<Args>(args)...));
		}
	};
}