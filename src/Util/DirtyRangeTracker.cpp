#include "DirtyRangeTracker.h"

DirtyRangeTracker::DirtyRangeTracker(uint16_t node_size) :
	m_node_size(node_size)
{}

void DirtyRangeTracker::AddItem(uint32_t pos)
{
	uint16_t node_index = pos / m_node_size;
	uint16_t pos_in_node = pos % m_node_size;

	Node& node = m_nodes[node_index];

	if (pos_in_node < node.first)
	{
		node.first = pos_in_node;
	}
	else if (pos_in_node > node.last)
	{
		node.last = pos_in_node;
	}
}

void DirtyRangeTracker::AddItems(uint32_t first, uint32_t last)
{
	uint16_t first_node_index = first / m_node_size;
	uint16_t start_in_first_node = first % m_node_size;

	uint16_t last_node_index = last / m_node_size;
	uint16_t end_in_last_node = last % m_node_size;

	if (first_node_index == last_node_index)
	{
		Node& node = m_nodes[first_node_index];

		if (start_in_first_node < node.first)
		{
			node.first = start_in_first_node;
		}
		else if (end_in_last_node > node.last)
		{
			node.last = end_in_last_node;
		}

		return;
	}

	uint16_t node_index = first_node_index;

	{
		Node& node = m_nodes[node_index];

		if (start_in_first_node < node.first)
		{
			node.first = start_in_first_node;
		}

		node.last = m_node_size - 1;
	}

	for (node_index++; node_index < last_node_index - 1; node_index++)
	{
		Node& node = m_nodes[node_index];

		node.first = 0;
		node.last = m_node_size - 1;
	}

	{
		Node& node = m_nodes[node_index];

		node.first = 0;

		if (end_in_last_node > node.last)
		{
			node.last = end_in_last_node;
		}
	}
}

void DirtyRangeTracker::Clear()
{
	m_nodes.clear();
}