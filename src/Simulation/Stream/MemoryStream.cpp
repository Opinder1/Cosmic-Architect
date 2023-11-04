#include "MemoryStream.h"

#include "Util/Debug.h"

#include <memory>

namespace sim
{
	MemoryStreamBase::MemoryStreamBase(char* data, size_t size) :
		m_data(data),
		m_size(size),
		m_stream_pos(0)
	{

	}

	size_t MemoryStreamBase::GetPos() const
	{
		return m_stream_pos;
	}

	void MemoryStreamBase::SetPos(size_t pos)
	{
		if (pos > m_size)
		{
			DEBUG_PRINT_ERROR("New stream position out of bounds");
			return;
		}

		m_stream_pos = pos;
	}

	void MemoryStreamBase::MovePos(int32_t offset)
	{
		if (m_stream_pos + offset > m_size)
		{
			DEBUG_PRINT_ERROR("New stream position out of bounds");
			return;
		}
		
		if (m_stream_pos < m_size)
		{
			DEBUG_PRINT_ERROR("New stream position out of bounds");
			return;
		}

		m_stream_pos += offset;
	}

	void MemoryStreamBase::SetEnd()
	{
		m_stream_pos = m_size - 1;
	}

	size_t MemoryStreamBase::Read(void* void_buffer, size_t count)
	{
		char* buffer = static_cast<char*>(void_buffer);

		if (buffer == nullptr)
		{
			DEBUG_PRINT_ERROR("The buffer shoud be a valid pointer");
			return 0;
		}

		if (count == 0) // If there is nothing to read then don't copy
		{
			DEBUG_PRINT_INFO("The read count was 0");
			return 0;
		}

		if (m_stream_pos + count > m_size)
		{
			DEBUG_PRINT_WARN("Buffer size greater than remaining stream size");
			count = m_size - m_stream_pos;
		}

		std::copy(m_data + m_stream_pos, m_data + m_stream_pos + count, buffer);
		m_stream_pos += count;

		return count;
	}

	void MemoryStreamBase::Flush()
	{
		DEBUG_PRINT_WARN("Memory streams do not support flushing");
	}

	bool MemoryStreamBase::CanExtend() const
	{
		return false;
	}

	size_t MemoryStreamBase::GetSize()
	{
		return m_size;
	}

	char* MemoryStreamBase::GetData() const
	{
		return m_data;
	}

	size_t MemoryStreamBase::GetStreamPos() const
	{
		return m_stream_pos;
	}

	void MemoryStreamBase::ForwardStreamPos(size_t offset)
	{
		m_stream_pos += offset;
	}

	MemoryStream::MemoryStream(char* data, size_t size) :
		MemoryStreamBase(data, size)
	{}

	size_t MemoryStream::Write(const void* void_buffer, size_t count)
	{
		const char* buffer = static_cast<const char*>(void_buffer);

		if (buffer == nullptr)
		{
			DEBUG_PRINT_ERROR("The buffer shoud be a valid pointer");
			return 0;
		}

		if (count == 0) // If there is nothing to write then do nothing
		{
			DEBUG_PRINT_INFO("The write count was 0");
			return 0;
		}

		if (GetStreamPos() + count > GetSize())
		{
			DEBUG_PRINT_WARN("Buffer size greater than remaining stream size");
			return 0;
		}

		std::copy(buffer, buffer + count, GetData() + GetStreamPos());

		ForwardStreamPos(count);

		return 0;
	}

	bool MemoryStream::CanWrite() const
	{
		return true;
	}

	char* MemoryStream::GetData() const
	{
		return MemoryStreamBase::GetData();
	}

	ConstMemoryStream::ConstMemoryStream(const char* data, size_t size) :
		MemoryStreamBase(const_cast<char*>(data), size)
	{}

	size_t ConstMemoryStream::Write(const void* buffer, size_t count)
	{
		DEBUG_PRINT_WARN("Memory streams are read only");
		return 0;
	}

	bool ConstMemoryStream::CanWrite() const
	{
		return false;
	}

	const char* ConstMemoryStream::GetData() const
	{
		return MemoryStreamBase::GetData();
	}
}