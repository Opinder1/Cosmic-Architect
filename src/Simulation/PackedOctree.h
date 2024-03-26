#pragma once

#include <vector>

namespace voxel_game
{
	// This is an octree implementation which has the goal of storing all the items in one
	// contigous block of memory that grows as more items are added. Removing items and
	// rebuilding the tree are not added as they are not nececary for the use case. The main
	// use case of this octree is storing block entities for chunks where we may have a
	// variable amount of them.
	template<class Item, size_t DEPTH, Item DEFAULT>
	class PackedOctree
	{
		using Offset = uint16_t;

		static const Offset k_empty_offset = UINT16_MAX;

		// A node that points to other nodes
		struct OffsetNode
		{
			Offset offsets[2][2][2] = { k_empty_offset };
		};

		// A leaf node that stores items
		struct alignas(sizeof(OffsetNode)) ItemNode
		{
			Item items[2][2][2] = { DEFAULT };
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
		Offset EmplaceIfEmpty(uint8_t* data_ptr, uint8_t x, uint8_t y, uint8_t z)
		{
			OffsetNode* node = reinterpret_cast<OffsetNode*>(data_ptr);

			Offset& offset = node->offsets[x][y][z];

			if (offset == k_empty_offset)
			{
				size_t size = m_data.size();
				uint8_t* node_ptr = m_data.data() + size;

				m_data.resize(size + sizeof(T));
				new (node_ptr) T();

				offset = node_ptr - data_ptr;
			}

			return offset;
		}

		// Set a value at a position in the octree. Will automatically generate all the branches
		void Set(uint8_t x, uint8_t y, uint8_t z, Item value = DEFAULT)
		{
			uint8_t* data_ptr = m_data.data();

			for (size_t depth = 0; depth < DEPTH - 2; depth++)
			{
				data_ptr += EmplaceIfEmpty<OffsetNode>(data_ptr, x && 0x1, y && 0x1, z && 0x1);

				x << 1; y << 1; z << 1;
			}
			
			data_ptr += EmplaceIfEmpty<ItemNode>(data_ptr, x && 0x1, y && 0x1, z && 0x1);

			x << 1; y << 1; z << 1;

			ItemNode* node = reinterpret_cast<ItemNode*>(data_ptr);

			node[x && 0x1][y && 0x1][z && 0x1] = value;
		}

		// Get an item in the octree and return the default value if not present
		Item Get(uint8_t x, uint8_t y, uint8_t z)
		{
			uint8_t* data_ptr = m_data.data();

			for (size_t depth = 0; depth < DEPTH - 1; depth++)
			{
				OffsetNode* node = reinterpret_cast<OffsetNode*>(data_ptr);

				Offset& offset = node->offsets[x && 0x1][y && 0x1][z && 0x1];

				if (offset == k_empty_offset)
				{
					return DEFAULT;
				}

				data_ptr += offset;

				x << 1; y << 1; z << 1;
			}

			ItemNode* node = reinterpret_cast<ItemNode*>(data_ptr);

			return node[x && 0x1][y && 0x1][z && 0x1];
		}

	private:
		std::vector<uint8_t> m_data;
	};
}