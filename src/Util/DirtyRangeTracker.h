#pragma once

#include <robin_hood/robin_hood.h>

#include <vector>
#include <cstdint>

class DirtyRangeTracker
{
	struct Node
	{
		uint16_t first = UINT16_MAX;
		uint16_t last = 0;
	};

public:
	DirtyRangeTracker(uint16_t node_size);

	void AddItem(uint32_t pos);

	void AddItems(uint32_t first, uint32_t last);

	template<class Callable>
	void ForEachRange(Callable&& callback)
	{
		bool previous_node_touches = false;
		uint32_t multinode_range_begin = 0;
		uint32_t multinode_range_end = 0;

		for (auto&& [node_index, node] : m_nodes)
		{
			uint32_t node_begin = node_index * m_node_size;

			bool touches_previous_node = node.first == 0;
			bool touches_next_node = node.last == m_node_size;

			if (previous_node_touches)
			{
				if (touches_previous_node) 
				{
					if (touches_next_node)
					{
						multinode_range_end = node_begin + node.last; // Continue the multinode range
						// previous_node_touches = true;
					}
					else
					{
						callback(multinode_range_begin, node_begin + node.last); // Do the range just for this node
						previous_node_touches = false;
					}
				}
				else
				{
					callback(multinode_range_begin, multinode_range_end); // Finish the previous multinode range

					if (touches_next_node)
					{
						multinode_range_begin = node_begin + node.first; // Start a new multinode range
						multinode_range_end = node_begin + node.last;
						// previous_node_touches = true;
					}
					else
					{
						callback(node_begin + node.first, node_begin + node.last); // Do the range just for this node
						previous_node_touches = false;
					}
				}
			}
			else
			{
				if (touches_next_node)
				{
					multinode_range_begin = node_begin + node.first; // Start a new multinode range
					multinode_range_end = node_begin + node.last;
					previous_node_touches = true;
				}
				else
				{
					callback(node_begin + node.first, node_begin + node.last); // Do the range just for this node
					// previous_node_touches = false;
				}
			}
		}
	}

	void Clear();

private:
	uint16_t m_node_size;
	robin_hood::unordered_node_map<uint32_t, Node> m_nodes;
};