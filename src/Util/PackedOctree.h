#pragma once

#include <godot_cpp/variant/vector3i.hpp>

#include <vector>

namespace voxel_game
{
	// This is an octree implementation which has the goal of storing all the items in one
	// contigous block of memory that grows as more items are added. The tree will not reduce in
	// size automatically when removing elements so ShrinkToFit should be called occasionally.
	// The main use case of this octree is storing block entities for chunks where we may have a
	// variable amount of them but we want to use as little memory as possible when we have a
	// Small amount of elements in the tree.
	template<class Item, size_t k_depth, Item k_invalid_item>
	class PackedOctree
	{
		using Offset = uint16_t;

		static const Offset k_invalid_offset = UINT16_MAX;

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
		PackedOctree()
		{
			m_data.resize(sizeof(OffsetNode));
			new (m_data.data()) OffsetNode();
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

		// Set a value at a position in the octree. Will automatically generate all the branches
		void Set(godot::Vector3i pos, Item value = k_invalid_item)
		{
			if (m_data.size() == 0)
			{
				if (value == k_invalid_item) // Don't bother emplacing and continuing if the item is invalid anyway
				{
					return;
				}

				EmplaceNode<OffsetNode>();
			}

			Offset offset = 0;

			for (size_t depth = 0; depth < k_depth - 1; depth++)
			{
				Offset child_offset = GetOffsetNode(offset).offsets[pos.x & 0x1][pos.y & 0x1][pos.z & 0x1];

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

					GetOffsetNode(offset).offsets[pos.x & 0x1][pos.y & 0x1][pos.z & 0x1] = child_offset;
				}
				offset += child_offset;

				pos.x >>= 1; pos.y >>= 1; pos.z >>= 1;
			}

			GetItemNode(offset).items[pos.x & 0x1][pos.y & 0x1][pos.z & 0x1] = value;
		}

		// Get an item in the octree and return the default value if not present
		Item Get(godot::Vector3i pos) const
		{
			const uint8_t* data_ptr = m_data.data();

			for (size_t depth = 0; depth < k_depth - 1; depth++)
			{
				const Offset& offset = reinterpret_cast<const OffsetNode*>(data_ptr)->offsets[pos.x & 0x1][pos.y & 0x1][pos.z & 0x1];

				if (offset == k_invalid_offset)
				{
					return k_invalid_item;
				}

				data_ptr += offset;

				pos.x >>= 1; pos.y >>= 1; pos.z >>= 1;
			}

			return reinterpret_cast<const ItemNode*>(data_ptr)->items[pos.x & 0x1][pos.y & 0x1][pos.z & 0x1];
		}

		// Clear the octree
		void Clear()
		{
			m_data.clear();
		}

		// Iterate over all the items in the octree
		template<class Callable>
		void Iterate(Callable callable) const
		{
			if (m_data.size() == 0)
			{
				return;
			}

			IterateBranch<Callable, 0>(m_data.data(), godot::Vector3i(0, 0, 0), callable);
		}

		// Rebuild the octree by rebalancing the tree and removing all empty items and branches
		void ShrinkToFit()
		{
			PackedOctree<Item, k_depth, k_invalid_item> new_octree;

			Iterate([&new_octree](godot::Vector3i pos, Item item)
			{
				new_octree.Set(pos, item);
			});

			m_data.swap(new_octree.m_data);
		}

	private:
		OffsetNode& GetOffsetNode(Offset offset)
		{
			return reinterpret_cast<OffsetNode&>(m_data[offset]);
		}

		ItemNode& GetItemNode(Offset offset)
		{
			return reinterpret_cast<ItemNode&>(m_data[offset]);
		}

		template<class Callable, size_t DEPTH>
		void IterateBranch(const uint8_t* data_ptr, godot::Vector3i pos, Callable callable) const
		{
			const OffsetNode* node = reinterpret_cast<const OffsetNode*>(data_ptr);

			for (uint8_t x = 0; x < 2; x++)
			for (uint8_t y = 0; y < 2; y++)
			for (uint8_t z = 0; z < 2; z++)
			{
				Offset offset = node->offsets[x][y][z];

				if (offset == k_invalid_offset)
				{
					continue;
				}

				godot::Vector3i pos_offset = godot::Vector3i(x << DEPTH, y << DEPTH, z << DEPTH);

				if constexpr (DEPTH == k_depth - 2)
				{
					IterateLeaf<Callable>(data_ptr + offset, pos + pos_offset, callable);
				}
				else
				{
					IterateBranch<Callable, DEPTH + 1>(data_ptr + offset, pos + pos_offset, callable);
				}
			}
		}

		template<class Callable>
		void IterateLeaf(const uint8_t* data_ptr, godot::Vector3i pos, Callable callable) const
		{
			const ItemNode* node = reinterpret_cast<const ItemNode*>(data_ptr);

			for (uint8_t x = 0; x < 2; x++)
			for (uint8_t y = 0; y < 2; y++)
			for (uint8_t z = 0; z < 2; z++)
			{
				Item item = node->items[x][y][z];

				if (item == k_invalid_item)
				{
					continue;
				}

				godot::Vector3i pos_offset = godot::Vector3i(x << (k_depth - 1), y << (k_depth - 1), z << (k_depth - 1));

				callable(pos + pos_offset, item);
			}
		}

	private:
		std::vector<uint8_t> m_data;
	};
}