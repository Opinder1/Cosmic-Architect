#pragma once

#include "Nocopy.h"
#include "Debug.h"

#include <utility>
#include <vector>
#include <tuple>

namespace voxel_game
{
	template<class Type>
	struct get_method_class;

	template<class Class, class Ret, class... Args>
	struct get_method_class<Ret(Class::*)(Args...)>
	{
		using type = Class;
	};

	using TypedCommand = const std::byte* (*)(void*, const std::byte*, const std::byte*, bool);

	class TypedCommandBuffer : Nocopy
	{
	public:
		using Storage = std::vector<std::byte>;

	public:
		TypedCommandBuffer();
		~TypedCommandBuffer();

		TypedCommandBuffer(TypedCommandBuffer&& other) noexcept;

		TypedCommandBuffer& operator=(TypedCommandBuffer&& other) noexcept;

		// Register a new command for the queue

		template<auto Method, class... Args>
		void AddCommand(Args&&... p_args);

		// Process only up to a certain number of commands and return how many were processed (0 for max to process all)
		size_t ProcessCommands(void* object, size_t max = 0);

		size_t NumCommands() const;

		// Destroy all remaining commands and arguments
		void Clear();

		void ShrinkToFit();

	private:
		Storage m_data;
		size_t m_start = 0;
		size_t m_num_commands = 0;
	};

	// Write a type to a buffer
	template<class T>
	const std::byte* ReadType(const std::byte* buffer_pos, const std::byte* buffer_end, const T*& value)
	{
		DEBUG_ASSERT(buffer_pos + sizeof(T) <= buffer_end, "Not enough space to read another encoded variant value");

		value = reinterpret_cast<const T*>(buffer_pos);

		return buffer_pos + sizeof(T);
	}

	// Write a type to a buffer
	template<class T>
	const std::byte* DestroyType(const std::byte* buffer_pos, const std::byte* buffer_end)
	{
		DEBUG_ASSERT(buffer_pos + sizeof(T) <= buffer_end, "Not enough space to read another encoded variant value");

		const T* value = reinterpret_cast<const T*>(buffer_pos);

		std::destroy_at(value);

		return buffer_pos + sizeof(T);
	}

	// Write a type to a buffer. We write the plain type while allowing pointers
	template<class T, class ArgT>
	void WriteType(TypedCommandBuffer::Storage& buffer, ArgT&& data)
	{
		size_t pos = buffer.size();
		buffer.resize(pos + sizeof(T));

		new (buffer.data() + pos) T(std::forward<ArgT>(data));
	}

	// A templated method wrapper that reads all arguments needed to call the method.
	template<auto Method, class Class, class... Args>
	struct DefaultCommand
	{
		// Storage type for arguments being stored in memory. A tuple is used to make sure alignment of members is correct
		using ArgStorage = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

		template<size_t... Indexes>
		static void Call(Class& object, const ArgStorage& args, std::index_sequence<Indexes...>)
		{
			(object.*Method)(std::get<Indexes>(args)...);
		}

		static const std::byte* Read(void* object, const std::byte* buffer_pos, const std::byte* buffer_end, bool execute)
		{
			if (execute)
			{
				const ArgStorage* args;
				ReadType<ArgStorage>(buffer_pos, buffer_end, args);

				Call(*static_cast<Class*>(object), *args, std::index_sequence_for<Args...>{});
			}

			return DestroyType<ArgStorage>(buffer_pos, buffer_end);
		}

		static void Write(TypedCommandBuffer::Storage& storage, Args&&... args)
		{
			WriteType<TypedCommand>(storage, &DefaultCommand::Read);

			WriteType<ArgStorage>(storage, ArgStorage(std::forward<Args>(args)...));
		}
	};

	template<auto Method, class... Args>
	void TypedCommandBuffer::AddCommand(Args&&... args)
	{
		DEBUG_ASSERT(m_start == 0, "We shouldn't be adding commands when we are already processing the buffer");

		using Class = typename get_method_class<decltype(Method)>::type;

		DefaultCommand<Method, Class, Args...>::Write(m_data, std::forward<Args>(args)...);

		m_num_commands++;
	}
}