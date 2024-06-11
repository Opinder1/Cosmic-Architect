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
		for (auto&& [node_index, node] : m_nodes)
		{
			uint32_t node_begin = node_index * m_node_size;

			callback(node_begin + node.first, node_begin + node.last);
		}

		m_nodes.clear();
	}

private:
	uint16_t m_node_size;
	robin_hood::unordered_node_map<uint32_t, Node> m_nodes;
};