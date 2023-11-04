#include "ByteArrayStream.h"

#include "Util/Debug.h"

#include <memory>

namespace sim
{
	ByteArrayStream::ByteArrayStream() :
		m_stream_pos(0)
	{

	}

	ByteArrayStream::ByteArrayStream(const godot::PackedByteArray& data) :
		m_data(data),
		m_stream_pos(0)
	{

	}

	size_t ByteArrayStream::GetPos() const
	{
		return m_stream_pos;
	}

	void ByteArrayStream::SetPos(size_t pos)
	{
		if (pos > m_data.size())
		{
			DEBUG_PRINT_ERROR("New stream position out of bounds");
			return;
		}
		
		m_stream_pos = pos;
	}

	void ByteArrayStream::MovePos(int32_t offset)
	{
		if (m_stream_pos + offset > m_data.size())
		{
			DEBUG_PRINT_ERROR("New stream position out of bounds");
			return;
		} 
		
		if (m_stream_pos < m_data.size())
		{
			DEBUG_PRINT_ERROR("New stream position out of bounds");
			return;
		}

		m_stream_pos += offset;
	}

	void ByteArrayStream::SetEnd()
	{
		m_stream_pos = m_data.size() - 1;
	}

	size_t ByteArrayStream::Read(void* void_buffer, size_t count)
	{
		char* buffer = static_cast<char*>(void_buffer);
		if (buffer == nullptr)
		{
			DEBUG_PRINT_ERROR("The buffer should be a valid pointer");
			return 0;
		}

		if (count == 0) // If there is nothing to read then don't copy
		{
			DEBUG_PRINT_INFO("The read count was 0");
			return 0;
		}

		if (m_stream_pos + count > m_data.size())
		{
			DEBUG_PRINT_WARN("Buffer size greater than remaining stream size");
			count = m_data.size() - m_stream_pos;
		}

		std::copy(m_data.ptr() + m_stream_pos, m_data.ptr() + m_stream_pos + count, buffer);
		m_stream_pos += count;

		return count;
	}

	size_t ByteArrayStream::Write(const void* void_buffer, size_t count)
	{
		const char* buffer = static_cast<const char*>(void_buffer);
		if (buffer == nullptr)
		{
			DEBUG_PRINT_ERROR("The buffer should be a valid pointer");
			return 0;
		}

		if (count == 0) // If there is nothing to write then do nothing
		{
			DEBUG_PRINT_INFO("The write count was 0");
			return 0;
		}

		m_data.resize(std::max(size_t(m_data.size()), m_stream_pos + count));
		std::copy(buffer, buffer + count, m_data.ptrw() + m_stream_pos);

		m_stream_pos += count;

		return count;
	}

	void ByteArrayStream::Flush()
	{
		DEBUG_PRINT_WARN("Vector streams do not support flushing");
	}

	bool ByteArrayStream::CanExtend() const
	{
		return true;
	}

	bool ByteArrayStream::CanWrite() const
	{
		return true;
	}

	size_t ByteArrayStream::GetSize()
	{
		return m_data.size();
	}

	void ByteArrayStream::SetData(const godot::PackedByteArray& data)
	{
		m_data = data;
		m_stream_pos = 0;
	}

	godot::PackedByteArray ByteArrayStream::GetData() const
	{
		return m_data;
	}
}