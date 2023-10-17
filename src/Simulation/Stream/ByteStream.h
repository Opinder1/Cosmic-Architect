#pragma once

#include "Util/Nocopy.h"

#include <cstdint>

namespace sim
{
	// A group of bytes that exists somewhere whether it be on a disk, in an archive or in memory.
	class ByteStream : nocopy
	{
	public:
		enum class GroupMagnitude
		{
			Short,
			Medium,
			Long,
		};

	public:
		ByteStream();

		virtual ~ByteStream();

		// Get the position of the stream pointer
		virtual size_t GetPos() const = 0;

		// Set the position of the stream pointer
		virtual void SetPos(size_t pos) = 0;

		// Move the position of the stream pointer
		virtual void MovePos(int32_t offset) = 0;

		// Set the position of the stream pointer to the end
		virtual void SetEnd() = 0;

		// Read bytes from the stream
		virtual size_t Read(void* buffer, size_t count) = 0;

		// Write bytes to the stream
		virtual size_t Write(const void* buffer, size_t count) = 0;

		// Apply the changes of this byte stream if they are not immediately applied
		virtual void Flush() = 0;

		// Return true if its possible for Write() to extend the stream length
		virtual bool CanExtend() const = 0;

		// Return true if it is possible to Write() to this stream
		virtual bool CanWrite() const = 0;

		// Get the size of the current stream buffer
		// TIP: Initial implementation slow so override if possible
		virtual size_t GetSize();
	};
}