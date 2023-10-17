#include "ByteStream.h"

namespace sim
{
	ByteStream::ByteStream()
	{

	}

	ByteStream::~ByteStream()
	{

	}

	size_t ByteStream::GetSize()
	{
		// Save original position.
		size_t original_pos = GetPos();

		// Go to end of the stream so we can query the position there
		SetEnd();

		// Get the position of the stream which should be the end
		size_t size_found = GetPos();

		// Restore original pos (Don't care if we fail)
		SetPos(original_pos);

		return size_found;
	}
}