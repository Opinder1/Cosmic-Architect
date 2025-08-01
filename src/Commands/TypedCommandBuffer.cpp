#include "TypedCommandBuffer.h"

#include "Util/Debug.h"
#include "Util/StackAllocator.h"

namespace voxel_game
{
	std::byte* ProcessCommand(void* object, std::byte* buffer_pos, std::byte* buffer_end, bool execute)
	{
		TCommand* command;
		buffer_pos = ReadType<TCommand>(buffer_pos, buffer_end, command);

		return (*command)(object, buffer_pos, buffer_end, execute);
	}

	TCommandBufferBase::TCommandBufferBase()
	{
		m_data.reserve(k_starting_buffer_size);
	}

	TCommandBufferBase::~TCommandBufferBase()
	{
		Clear();
	}

	TCommandBufferBase::TCommandBufferBase(TCommandBufferBase&& other) noexcept
	{
		*this = std::move(other);
	}

	TCommandBufferBase& TCommandBufferBase::operator=(TCommandBufferBase&& other) noexcept
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

	size_t TCommandBufferBase::ProcessCommandsUntyped(void* object, size_t max)
	{
		DEBUG_ASSERT(object != nullptr, "The object we are trying to process on should be valid");

		if (m_num_commands == 0)
		{
			return 0;
		}

		size_t num_processed = 0;

		std::byte* buffer_start = m_data.data() + m_start;
		std::byte* buffer_pos = buffer_start;
		std::byte* buffer_end = m_data.data() + m_data.size();

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

	size_t TCommandBufferBase::NumCommands() const
	{
		return m_num_commands;
	}

	void TCommandBufferBase::Clear()
	{
		// Go through the buffer and read command data as if we were processing the commands but only destroy the data

		std::byte* buffer_pos = m_data.data() + m_start;
		std::byte* buffer_end = m_data.data() + m_data.size();
		while (buffer_pos != buffer_end)
		{
			buffer_pos = ProcessCommand(nullptr, buffer_pos, buffer_end, false);
		}

		m_data.clear();
		m_start = 0;
		m_num_commands = 0;
	}

	void TCommandBufferBase::ShrinkToFit()
	{
		m_data.shrink_to_fit();
	}
}