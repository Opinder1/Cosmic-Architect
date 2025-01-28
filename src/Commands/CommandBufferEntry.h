#pragma once

#include "Util/Nocopy.h"

#include <godot_cpp/classes/object.hpp>

namespace voxel_game
{
	constexpr const size_t k_starting_buffer_size = 4096;
	constexpr const size_t k_process_all_commands = 0;

	class CommandBufferEntryBase : Nocopy
	{
	public:
		CommandBufferEntryBase() {}

		virtual size_t ProcessCommands(godot::Object* object, size_t max = k_process_all_commands) = 0;

		virtual size_t NumCommands() const = 0;

		virtual void Clear() = 0;

		virtual void ShrinkToFit() = 0;
	};

	template<class T>
	class CommandBufferEntry final : public CommandBufferEntryBase
	{
	public:
		CommandBufferEntry(T&& buffer) :
			m_buffer(std::move(buffer))
		{}

		size_t ProcessCommands(godot::Object* object, size_t max) override
		{
			return m_buffer.ProcessCommands(object, max);
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