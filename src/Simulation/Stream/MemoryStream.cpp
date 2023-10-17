#include "MemoryStream.h"

#include <godot_cpp/core/error_macros.hpp>

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
		ERR_FAIL_COND_MSG(pos > m_size, "New stream position out of bounds");

		m_stream_pos = pos;
	}

	void MemoryStreamBase::MovePos(int32_t offset)
	{
		ERR_FAIL_COND_MSG(m_stream_pos + offset > m_size || m_stream_pos < m_size, "New stream position out of bounds");

		m_stream_pos += offset;
	}

	void MemoryStreamBase::SetEnd()
	{
		m_stream_pos = m_size - 1;
	}

	size_t MemoryStreamBase::Read(void* void_buffer, size_t count)
	{
		char* buffer = static_cast<char*>(void_buffer);

		ERR_FAIL_NULL_V_MSG(buffer, 0, "The buffer shoud be a valid pointer");

		if (m_stream_pos + count > m_size)
		{
			WARN_PRINT("Buffer size greater than remaining stream size");
			count = m_size - m_stream_pos;
		}

		// If there is nothing to read then don't copy
		ERR_FAIL_COND_V_MSG(count == 0, 0, "The read count was 0");

		std::copy(m_data + m_stream_pos, m_data + m_stream_pos + count, buffer);
		m_stream_pos += count;

		return count;
	}

	void MemoryStreamBase::Flush()
	{
		WARN_PRINT("Memory streams do not support flushing");
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
		ERR_FAIL_NULL_V_MSG(buffer, 0, "the buffer should be a valid pointer");

		ERR_FAIL_COND_V_MSG(count == 0, 0, "The write count was 0");

		ERR_FAIL_COND_V_MSG(GetStreamPos() + count > GetSize(), 0, "Buffer size greater than remaining stream size");

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
		ERR_FAIL_V_MSG(0, "Memory streams are read only");
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