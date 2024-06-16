#include "DirtyTracker.h"

DirtyRangeTracker::DirtyRangeTracker(uint16_t chunk_size) :
	m_chunk_size(chunk_size)
{}

void DirtyRangeTracker::AddItem(uint32_t pos)
{
	uint16_t chunk_index = pos / m_chunk_size;
	uint16_t pos_in_chunk = pos % m_chunk_size;

	Chunk& chunk = m_chunks[chunk_index];

	if (pos_in_chunk < chunk.first)
	{
		chunk.first = pos_in_chunk;
	}
	else if (pos_in_chunk > chunk.last)
	{
		chunk.last = pos_in_chunk;
	}
}

void DirtyRangeTracker::AddItems(uint32_t first, uint32_t last)
{
	uint16_t first_chunk_index = first / m_chunk_size;
	uint16_t start_in_first_chunk = first % m_chunk_size;

	uint16_t last_chunk_index = last / m_chunk_size;
	uint16_t end_in_last_chunk = last % m_chunk_size;

	if (first_chunk_index == last_chunk_index)
	{
		Chunk& chunk = m_chunks[first_chunk_index];

		if (start_in_first_chunk < chunk.first)
		{
			chunk.first = start_in_first_chunk;
		}
		else if (end_in_last_chunk > chunk.last)
		{
			chunk.last = end_in_last_chunk;
		}

		return;
	}

	uint16_t chunk_index = first_chunk_index;

	{
		Chunk& chunk = m_chunks[chunk_index];

		if (start_in_first_chunk < chunk.first)
		{
			chunk.first = start_in_first_chunk;
		}

		chunk.last = m_chunk_size - 1;
	}

	for (chunk_index++; chunk_index < last_chunk_index - 1; chunk_index++)
	{
		Chunk& chunk = m_chunks[chunk_index];

		chunk.first = 0;
		chunk.last = m_chunk_size - 1;
	}

	{
		Chunk& chunk = m_chunks[chunk_index];

		chunk.first = 0;

		if (end_in_last_chunk > chunk.last)
		{
			chunk.last = end_in_last_chunk;
		}
	}
}

void DirtyRangeTracker::Clear()
{
	m_chunks.clear();
}

DirtyChunkTracker::DirtyChunkTracker(uint16_t chunk_size) :
	m_chunk_size(chunk_size)
{}

void DirtyChunkTracker::AddItem(uint32_t pos)
{
	uint16_t chunk_index = pos / m_chunk_size;

	m_chunks.emplace(chunk_index);
}

void DirtyChunkTracker::AddItems(uint32_t first, uint32_t last)
{
	uint16_t first_chunk_index = first / m_chunk_size;

	uint16_t last_chunk_index = last / m_chunk_size;

	for (uint16_t chunk_index = first_chunk_index; chunk_index != last_chunk_index; chunk_index++)
	{
		m_chunks.emplace(chunk_index);
	}
}

void DirtyChunkTracker::Clear()
{
	m_chunks.clear();
}