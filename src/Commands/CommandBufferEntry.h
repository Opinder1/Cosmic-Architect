#pragma once

#include "Util/Nocopy.h"

#include <godot_cpp/classes/object.hpp>

#include <vector>

namespace voxel_game
{
	constexpr const size_t k_starting_buffer_size = 4096;
	constexpr const size_t k_process_all_commands = 0;

	using Storage = std::vector<std::byte>;

	// Write a type to a buffer. We write the plain type while allowing pointers
	template<class T, class ArgT>
	void WriteType(Storage& buffer, ArgT&& data)
	{
		size_t pos = buffer.size();
		buffer.resize(pos + sizeof(T));

		new (buffer.data() + pos) T(std::forward<ArgT>(data));
	}

	class CommandBufferEntryBase : Nocopy
	{
	public:
		CommandBufferEntryBase() {}

		virtual size_t ProcessCommands(void* object, size_t max = k_process_all_commands) = 0;

		virtual size_t NumCommands() const = 0;

		virtual void Clear() = 0;

		virtual void ShrinkToFit() = 0;
	};

	template<class T>
	class TCommandBufferEntry final : public CommandBufferEntryBase
	{
	public:
		TCommandBufferEntry(T&& buffer) :
			m_buffer(std::move(buffer))
		{}

		size_t ProcessCommands(void* object, size_t max) override
		{
			return m_buffer.ProcessCommandsUntyped(object, max);
		}

		size_t NumCommands() const override
		{
			return m_buffer.NumCommands();
		}

		void Clear() override
		{
			m_buffer.Clear();
		}

		void ShrinkToFit() override
		{
			m_buffer.ShrinkToFit();
		}

	private:
		T m_buffer;
	};
}