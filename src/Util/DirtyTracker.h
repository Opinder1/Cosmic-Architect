#pragma once

#include <set>
#include <map>
#include <cstdint>

// A tracker for telling which ranges of data have been modified aligned to chunks
class DirtyRangeTracker
{
	struct Chunk
	{
		uint16_t first = UINT16_MAX;
		uint16_t last = 0;
	};

public:
	DirtyRangeTracker(uint16_t chunk_size);

	void AddItem(uint32_t pos);

	void AddItems(uint32_t first, uint32_t last);

	template<class Callable>
    void ForEachRange(Callable&& callback);

	void Clear();

private:
	uint16_t m_chunk_size;
	std::map<uint16_t, Chunk> m_chunks;
};

// A tracker for telling which chunks of data have been modified
class DirtyChunkTracker
{
public:
    DirtyChunkTracker(uint16_t chunk_size);

    void AddItem(uint32_t pos);

    void AddItems(uint32_t first, uint32_t last);

    template<class Callable>
    void ForEachRange(Callable&& callback);

    void Clear();

private:
    uint16_t m_chunk_size;
    std::set<uint16_t> m_chunks;
};

template<class Callable>
void DirtyRangeTracker::ForEachRange(Callable&& callback)
{
    uint32_t multichunk_range_begin = UINT32_MAX;
    uint32_t multichunk_range_end = UINT32_MAX;
    uint32_t previous_chunk_end = UINT32_MAX - 1;

    for (auto&& [chunk_index, chunk] : m_chunks)
    {
        uint32_t chunk_begin = chunk_index * m_chunk_size;

        bool touches_next_chunk = chunk.last == (m_chunk_size - 1);

        if (previous_chunk_end + 1 == chunk_begin + chunk.first) // Our chunk is sequentially after the previous
        {
            multichunk_range_end = chunk_begin + chunk.last;

            if (!touches_next_chunk) // If we don't touch the next chunk then we can finish the current range
            {
                callback(multichunk_range_begin, multichunk_range_end);
                multichunk_range_begin = UINT32_MAX;
            }
        }
        else
        {
            if (multichunk_range_begin != UINT32_MAX) // We were currently in a range but didn't touch the last chunk so finish the range
            {
                callback(multichunk_range_begin, multichunk_range_end);
                multichunk_range_begin = UINT32_MAX;
            }

            if (touches_next_chunk) // We don't touch the previous chunk but touch the next so start a new range
            {
                multichunk_range_begin = chunk_begin + chunk.first;
                multichunk_range_end = chunk_begin + chunk.last;
            }
            else // If we don't touch the next chunk then we can finish the current range
            {
                callback(chunk_begin + chunk.first, chunk_begin + chunk.last);
            }
        }

        previous_chunk_end = chunk_begin + chunk.last;
    }

    if (multichunk_range_begin != UINT32_MAX) // Finish the range if there is one in progress
    {
        callback(multichunk_range_begin, multichunk_range_end);
    }
}

template<class Callable>
void DirtyChunkTracker::ForEachRange(Callable&& callback)
{
    uint16_t multichunk_range_begin = UINT16_MAX;
    uint16_t multichunk_range_end = UINT16_MAX;
    uint16_t last_chunk_index = UINT16_MAX - 1;

    for (uint16_t chunk_index : m_chunks)
    {
        uint32_t chunk_begin = chunk_index * m_chunk_size;

        if (last_chunk_index + 1 == chunk_index) // Our chunk is sequentially after the previous so continue range
        {
            multichunk_range_end = chunk_index;
        }
        else
        {
            if (multichunk_range_begin != UINT16_MAX) // We don't touch the previous so finish the current chunk range
            {
                callback(multichunk_range_begin, multichunk_range_end);
            }

            // Start a new chunk range
            multichunk_range_begin = chunk_index;
            multichunk_range_end = chunk_index;
        }

        last_chunk_index = chunk_index;
    }

    if (multichunk_range_begin != UINT16_MAX) // We still have a range so finish it
    {
        callback(multichunk_range_begin, multichunk_range_end);
    }
}