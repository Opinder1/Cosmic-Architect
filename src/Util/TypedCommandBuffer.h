#pragma once

#include "Nocopy.h"
#include "Debug.h"

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
		size_t ProcessCommands(godot::Object* object, size_t max = 0);

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
	void WriteType(ArgT&& data, TypedCommandBuffer::Storage& buffer)
	{
		size_t pos = buffer.size();
		buffer.resize(pos + sizeof(T));

		new (buffer.data() + pos) T(std::forward<ArgT>(data));
	}

	// Storage type for arguments being stored in memory. A tuple is used to make sure alignment of members is correct
	template<class... Args>
	using PlainArgs = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

	// A templated method wrapper that reads all arguments needed to call the method. Each number of arguments is defined explicitly
	// because the order of argument evaluation is undefined.
	template<auto Method, class Class, class... Args>
	struct DefaultCommand;

	template<auto Method, class Class>
	struct DefaultCommand<Method, Class>
	{
		static const std::byte* Execute(void* object, const std::byte* buffer_pos, const std::byte* buffer_end, bool execute)
		{
			if (execute)
			{
				(static_cast<Class*>(object)->*Method)();
			}

			return buffer_pos;
		}
	};

	template<auto Method, class Class, class Arg1>
	struct DefaultCommand<Method, Class, Arg1>
	{
		using Args = PlainArgs<Arg1>;

		static const std::byte* Execute(void* object, const std::byte* buffer_pos, const std::byte* buffer_end, bool execute)
		{
			if (execute)
			{
				const Args* args;
				ReadType<Args>(buffer_pos, buffer_end, args);

				(static_cast<Class*>(object)->*Method)(std::get<0>(*args));
			}

			return DestroyType<Args>(buffer_pos, buffer_end);
		}
	};

	template<auto Method, class Class, class Arg1, class Arg2>
	struct DefaultCommand<Method, Class, Arg1, Arg2>
	{
		using Args = PlainArgs<Arg1, Arg2>;

		static const std::byte* Execute(void* object, const std::byte* buffer_pos, const std::byte* buffer_end, bool execute)
		{
			if (execute)
			{
				const Args* args;
				ReadType<Args>(buffer_pos, buffer_end, args);

				(static_cast<Class*>(object)->*Method)(std::get<0>(*args), std::get<1>(*args));
			}

			return DestroyType<Args>(buffer_pos, buffer_end);
		}
	};

	template<auto Method, class Class, class Arg1, class Arg2, class Arg3>
	struct DefaultCommand<Method, Class, Arg1, Arg2, Arg3>
	{
		using Args = PlainArgs<Arg1, Arg2, Arg3>;

		static const std::byte* Execute(void* object, const std::byte* buffer_pos, const std::byte* buffer_end, bool execute)
		{
			if (execute)
			{
				const Args* args;
				ReadType<Args>(buffer_pos, buffer_end, args);

				(static_cast<Class*>(object)->*Method)(std::get<0>(*args), std::get<1>(*args), std::get<2>(*args));
			}

			return DestroyType<Args>(buffer_pos, buffer_end);
		}
	};

	template<auto Method, class Class, class Arg1, class Arg2, class Arg3, class Arg4>
	struct DefaultCommand<Method, Class, Arg1, Arg2, Arg3, Arg4>
	{
		using Args = PlainArgs<Arg1, Arg2, Arg3, Arg4>;

		static const std::byte* Execute(void* object, const std::byte* buffer_pos, const std::byte* buffer_end, bool execute)
		{
			if (execute)
			{
				const Args* args;
				ReadType<Args>(buffer_pos, buffer_end, args);

				(static_cast<Class*>(object)->*Method)(std::get<0>(*args), std::get<1>(*args), std::get<2>(*args), std::get<3>(*args));
			}

			return DestroyType<Args>(buffer_pos, buffer_end);
		}
	};

	template<auto Method, class Class, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
	struct DefaultCommand<Method, Class, Arg1, Arg2, Arg3, Arg4, Arg5>
	{
		using Args = PlainArgs<Arg1, Arg2, Arg3, Arg4, Arg5>;

		static const std::byte* Execute(void* object, const std::byte* buffer_pos, const std::byte* buffer_end, bool execute)
		{
			if (execute)
			{
				const Args* args;
				ReadType<Args>(buffer_pos, buffer_end, args);

				(static_cast<Class*>(object)->*Method)(std::get<0>(*args), std::get<1>(*args), std::get<2>(*args), std::get<3>(*args), std::get<4>(*args));
			}

			return DestroyType<Args>(buffer_pos, buffer_end);
		}
	};

	template<auto Method, class... Args>
	void TypedCommandBuffer::AddCommand(Args&&... args)
	{
		DEBUG_ASSERT(m_start == 0, "We shouldn't be adding commands when we are already processing the buffer");

		using Class = typename get_method_class<decltype(Method)>::type;

		WriteType<TypedCommand>(&DefaultCommand<Method, Class, Args...>::Execute, m_data);

		WriteType<PlainArgs<Args...>>(PlainArgs<Args...>(std::forward<Args>(args)...), m_data);

		m_num_commands++;
	}
}