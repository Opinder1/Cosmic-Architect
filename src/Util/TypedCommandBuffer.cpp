#include "CommandBuffer.h"

#include "Util/Debug.h"
#include "Util/StackAllocator.h"

namespace voxel_game
{
	const std::byte* ProcessCommand(godot::Object* object, const std::byte* buffer_pos, const std::byte* buffer_end, bool execute)
	{
		const TypedCommand* command;
		buffer_pos = ReadType<TypedCommand>(buffer_pos, buffer_end, command);

		return (*command)(object, buffer_pos, buffer_end, execute);
	}

	TypedCommandBuffer::TypedCommandBuffer()
	{
		m_data.reserve(k_starting_buffer_size);
	}

	TypedCommandBuffer::~TypedCommandBuffer()
	{
		Clear();
	}

	TypedCommandBuffer::TypedCommandBuffer(TypedCommandBuffer&& other) noexcept
	{
		*this = std::move(other);
	}

	TypedCommandBuffer& TypedCommandBuffer::operator=(TypedCommandBuffer&& other) noexcept
	{
		Clear();

		m_data = std::move(other.m_data);
		m_num_commands = other.m_num_commands;
		m_start = other.m_start;

		other.m_data.reserve(k_starting_buffer_size);
		other.m_num_commands = 0;
		other.m_start = 0;

		return *this;
	}

	size_t TypedCommandBuffer::ProcessCommands(godot::Object* object, size_t max)
	{
		DEBUG_ASSERT(object != nullptr, "The object we are trying to process on should be valid");

		if (m_num_commands == 0)
		{
			return 0;
		}

		size_t num_processed = 0;

		const std::byte* buffer_start = m_data.data() + m_start;
		const std::byte* buffer_pos = buffer_start;
		const std::byte* buffer_end = m_data.data() + m_data.size();

		while (buffer_pos != buffer_end)
		{
			buffer_pos = ProcessCommand(object, buffer_pos, buffer_end, true);
			num_processed++;

			// We reached the max commands we set for ourself
			if (num_processed == max)
			{
				break;
			}
		}

		m_start += buffer_pos - buffer_start;
		DEBUG_ASSERT(m_num_commands >= num_processed, "We processed more commands than we have");
		m_num_commands -= num_processed;

		return num_processed;
	}

	size_t TypedCommandBuffer::NumCommands() const
	{
		return m_num_commands;
	}

	void TypedCommandBuffer::Clear()
	{
		// Go through the buffer and read command data as if we were processing the commands but only destroy the data

		const std::byte* buffer_pos = m_data.data() + m_start;
		const std::byte* buffer_end = m_data.data() + m_data.size();
		while (buffer_pos != buffer_end)
		{
			buffer_pos = ProcessCommand(nullptr, buffer_pos, buffer_end, false);
		}

		m_data.clear();
		m_start = 0;
		m_num_commands = 0;
	}

	void TypedCommandBuffer::ShrinkToFit()
	{
		m_data.shrink_to_fit();
	}
}