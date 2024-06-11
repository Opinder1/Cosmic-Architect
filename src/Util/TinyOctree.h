#pragma once

#include "VectorOperators.h"
#include "Debug.h"

#include <godot_cpp/variant/vector3i.hpp>

#include <vector>

// This is an octree implementation which has the goal of storing all the items in one
// contigous block of memory that grows as more items are added. The tree will not reduce in
// size automatically when removing elements so ShrinkToFit should be called occasionally.
// The main use case of this octree is storing block entities for chunks where we may have a
// variable amount of them but we want to use as little memory as possible when we have a
// Small amount of elements in the tree.
template<class Item, size_t k_depth, Item k_invalid_item>
class TinyOctree
{
	using Offset = uint16_t;

	static constexpr const Offset k_invalid_offset = UINT16_MAX;

	// A node that points to other nodes
	struct OffsetNode
	{
		Offset offsets[2][2][2] = { k_invalid_offset, k_invalid_offset, k_invalid_offset, k_invalid_offset,
									k_invalid_offset, k_invalid_offset, k_invalid_offset, k_invalid_offset };
	};

	// A leaf node that stores items
	struct alignas(sizeof(OffsetNode)) ItemNode
	{
		Item items[2][2][2] = { k_invalid_item, k_invalid_item, k_invalid_item, k_invalid_item,
								k_invalid_item, k_invalid_item, k_invalid_item, k_invalid_item };
	};

public:
	TinyOctree() {}

	// Set a value at a position in the octree. Will automatically generate all the branches
	void Set(godot::Vector3i pos, Item value = k_invalid_item)
	{
		if (!PosInOctree(pos))
		{
			DEBUG_PRINT_ERROR("We tried to set an item outside the bounds of the octree");
			return;
		}

		if (m_start_depth == k_depth)
		{
			DEBUG_ASSERT(m_data.size() == 0, "We shouldn't have any data allocated if we have a single item");

			if (m_single_item == k_invalid_item) // If the single item is invalid then we have 0 items
			{
				m_single_item = value;

				m_x_mask = pos.x;
				m_y_mask = pos.y;
				m_z_mask = pos.z;
				return;
			}

			if (value == k_invalid_item && PosInBounds(pos)) // If we have one item and we set it to invalid then clear it
			{
				m_single_item = k_invalid_item;

				m_x_mask = 0;
				m_y_mask = 0;
				m_z_mask = 0;
				return;
			}
		}

		while (!PosInBounds(pos)) // Keep adding new roots until the pos is in the tree
		{
			AddNewRoot();
		}

		if (m_start_depth == k_depth) // If we are in bounds and at max depth then we are changing our one value
		{
			DEBUG_ASSERT(m_data.size() == 0, "We shouldn't have any data allocated if we have a single item");
			m_single_item = value;
			return;
		}

		uint32_t bit = 0b1 << (k_depth - m_start_depth - 1);

		Offset offset = 0;

		for (size_t depth = m_start_depth; depth < k_depth - 1; depth++)
		{
			bool x = !!(pos.x & bit);
			bool y = !!(pos.y & bit);
			bool z = !!(pos.z & bit);

			Offset child_offset = GetOffsetNode(offset).offsets[x][y][z];

			if (child_offset == k_invalid_offset)
			{
				if (value == k_invalid_item) // Don't bother emplacing and continuing if the item is invalid anyway
				{
					return;
				}

				if (depth == k_depth - 2)
				{
					child_offset = EmplaceNode<ItemNode>() - offset;
				}
				else
				{
					child_offset = EmplaceNode<OffsetNode>() - offset;
				}

				GetOffsetNode(offset).offsets[x][y][z] = child_offset;
			}
			offset += child_offset;

			bit >>= 1;
		}

		bool x = !!(pos.x & bit);
		bool y = !!(pos.y & bit);
		bool z = !!(pos.z & bit);

		GetItemNode(offset).items[x][y][z] = value;
	}

	// Get an item in the octree and return the default value if not present
	Item Get(godot::Vector3i pos) const
	{
		if (!PosInOctree(pos))
		{
			DEBUG_PRINT_ERROR("We tried to get an item outside the bounds of the octree");
			return k_invalid_item;
		}

		if (!PosInBounds(pos))
		{
			return k_invalid_item;
		}

		if (m_start_depth == k_depth)
		{
			DEBUG_ASSERT(m_data.size() == 0, "We shouldn't have any data allocated if we have a single item");
			return m_single_item;
		}

		uint32_t bit = 0b1 << (k_depth - m_start_depth - 1);

		const uint8_t* data_ptr = m_data.data();

		for (size_t depth = m_start_depth; depth < k_depth - 1; depth++)
		{
			bool x = !!(pos.x & bit);
			bool y = !!(pos.y & bit);
			bool z = !!(pos.z & bit);

			const Offset& offset = reinterpret_cast<const OffsetNode*>(data_ptr)->offsets[x][y][z];

			if (offset == k_invalid_offset)
			{
				return k_invalid_item;
			}

			data_ptr += offset;

			bit >>= 1;
		}

		bool x = !!(pos.x & bit);
		bool y = !!(pos.y & bit);
		bool z = !!(pos.z & bit);

		return reinterpret_cast<const ItemNode*>(data_ptr)->items[x][y][z];
	}

	// Clear the octree
	void Clear()
	{
		*this = TinyOctree<Item, k_depth, k_invalid_item>();
	}

	// Iterate over all the items in the octree
	template<class Callable>
	void Iterate(Callable&& callable) const
	{
		godot::Vector3i pos(m_x_mask, m_y_mask, m_z_mask);

		if (m_start_depth == k_depth) // No allocations so check the single item
		{
			DEBUG_ASSERT(m_data.size() == 0, "We shouldn't have any data allocated if we have a single item");

			if (m_single_item != k_invalid_item)
			{
				callable(pos, m_single_item);
			}
		}
		else if (m_start_depth == k_depth - 1) // The root node is a leaf node
		{
			IterateLeaf<Callable>(m_data.data(), pos, callable);
		}
		else
		{
			IterateBranch<Callable>(m_data.data(), m_start_depth, pos, callable);
		}
	}

	// Rebuild the octree by rebalancing the tree and removing all empty items and branches
	void ShrinkToFit()
	{
		if (m_data.size() == 0) // Nothing to swap
		{
			DEBUG_ASSERT(m_start_depth == k_depth, "Our data should only be empty if we are starting at max depth");
			return;
		}

		DEBUG_ASSERT(m_start_depth < k_depth, "Our data should be allocated if we don't start at the max depth");

		// Make new octree which we will fill in with all our values
		TinyOctree<Item, k_depth, k_invalid_item> new_octree;

		Iterate([&new_octree](godot::Vector3i pos, Item item)
		{
			new_octree.Set(pos, item);
		});

		// Swap our data to the new octree
		std::swap(m_start_depth, new_octree.m_start_depth);
		std::swap(m_x_mask, new_octree.m_x_mask);
		std::swap(m_y_mask, new_octree.m_y_mask);
		std::swap(m_z_mask, new_octree.m_z_mask);
		m_data.swap(new_octree.m_data);
	}

private:
	bool PosInBounds(godot::Vector3i pos) const
	{
		uint32_t cut_off_bits = k_depth - m_start_depth;

		return (((pos.x >> cut_off_bits) << cut_off_bits) == m_x_mask &&
				((pos.y >> cut_off_bits) << cut_off_bits) == m_y_mask &&
				((pos.z >> cut_off_bits) << cut_off_bits) == m_z_mask);
	}

	bool PosInOctree(godot::Vector3i pos) const
	{
		return	(pos.x >= 0) && (pos.x < (1 << k_depth)) &&
				(pos.y >= 0) && (pos.y < (1 << k_depth)) &&
				(pos.z >= 0) && (pos.z < (1 << k_depth));
	}

	// Check a position of the node at the requested data pos and if its empty then create
	// a new node of the requested type
	template<class T>
	Offset EmplaceNode()
	{
		size_t old_size = m_data.size();
		size_t new_size = old_size + sizeof(T);

		m_data.resize(new_size);
		new (m_data.data() + old_size) T();

		return static_cast<Offset>(old_size);
	}

	void AddNewRoot()
	{
		DEBUG_ASSERT(m_start_depth > 0, "We expanded bounds too many times!");

		m_start_depth--;

		if (m_start_depth == k_depth - 1)
		{
			DEBUG_ASSERT(m_data.size() == 0, "We shouldn't have any data allocated if we have a single item");

			EmplaceNode<ItemNode>();

			// Add the previous single item
			GetItemNode(0).items[m_x_mask & 0x1][m_y_mask & 0x1][m_z_mask & 0x1] = m_single_item;
		}
		else
		{
			// Insert a new offset node at the begining to be the new root
			m_data.insert(m_data.begin(), sizeof(OffsetNode), 0);
			new (m_data.data()) OffsetNode();

			uint32_t bit = 0b1 << (k_depth - m_start_depth - 1);

			// Point the right item in the new root to the old root
			GetOffsetNode(0).offsets[!!(m_x_mask & bit)][!!(m_y_mask & bit)][!!(m_z_mask & bit)] = sizeof(OffsetNode);
		}

		// Update our depth and mask
		uint32_t cut_off_bits = k_depth - m_start_depth;
		m_x_mask = (m_x_mask >> cut_off_bits) << cut_off_bits;
		m_y_mask = (m_y_mask >> cut_off_bits) << cut_off_bits;
		m_z_mask = (m_z_mask >> cut_off_bits) << cut_off_bits;
	}

	OffsetNode& GetOffsetNode(Offset offset)
	{
		return reinterpret_cast<OffsetNode&>(m_data[offset]);
	}

	ItemNode& GetItemNode(Offset offset)
	{
		return reinterpret_cast<ItemNode&>(m_data[offset]);
	}

	template<class Callable>
	void IterateBranch(const uint8_t* data_ptr, uint32_t depth, godot::Vector3i pos, Callable&& callable) const
	{
		const OffsetNode* node = reinterpret_cast<const OffsetNode*>(data_ptr);

		uint32_t child_depth = depth + 1;

		for (uint32_t x = 0; x < 2; x++)
		for (uint32_t y = 0; y < 2; y++)
		for (uint32_t z = 0; z < 2; z++)
		{
			Offset offset = node->offsets[x][y][z];

			if (offset == k_invalid_offset)
			{
				continue;
			}

			godot::Vector3i pos_offset = godot::Vector3i(x, y, z) << (k_depth - depth - 1);

			if (child_depth == k_depth - 1)
			{
				IterateLeaf<Callable>(data_ptr + offset, pos + pos_offset, callable);
			}
			else
			{
				IterateBranch<Callable>(data_ptr + offset, child_depth, pos + pos_offset, callable);
			}
		}
	}

	template<class Callable>
	void IterateLeaf(const uint8_t* data_ptr, godot::Vector3i pos, Callable&& callable) const
	{
		const ItemNode* node = reinterpret_cast<const ItemNode*>(data_ptr);

		for (uint32_t x = 0; x < 2; x++)
		for (uint32_t y = 0; y < 2; y++)
		for (uint32_t z = 0; z < 2; z++)
		{
			Item item = node->items[x][y][z];

			if (item == k_invalid_item)
			{
				continue;
			}

			godot::Vector3i pos_offset = godot::Vector3i(x, y, z);
			callable(pos + pos_offset, item);
		}
	}

private:
	uint8_t m_start_depth = k_depth;
	uint8_t m_x_mask = 0;
	uint8_t m_y_mask = 0;
	uint8_t m_z_mask = 0;
	Item m_single_item = k_invalid_item;
	std::vector<uint8_t> m_data;
};