#pragma once

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/templates/rid_owner.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <memory>
#include <algorithm>

namespace voxel_game
{
	// RID object allocation system where all the objects are stored in a contiguous array
	template<class T>
	class RIDContiguousOwner
	{
		constexpr static size_t k_item_size = sizeof(T) / sizeof(float);

	public:
		RIDContiguousOwner()
		{
			m_buffer.resize(k_item_size * 8);
		}

		size_t Size()
		{
			return m_item_offsets.get_rid_count();
		}

		bool IsEmpty()
		{
			return Size() == 0;
		}

		godot::RID Add(const T& input)
		{
			uint64_t offset = m_buffer.size();
			m_buffer.resize(offset + k_item_size);

			T* item = reinterpret_cast<T*>(m_buffer.ptrw() + offset);

			*item = input;

			return m_item_offsets.make_rid(item);
		}

		T* Get(godot::RID id)
		{
			return m_item_offsets.get_or_null(id);
		}

		void Remove(godot::RID id)
		{
			T* target_item = m_item_offsets.get_or_null(id);

			if (target_item == nullptr)
			{
				return;
			}

			uint32_t count = m_item_offsets.get_rid_count();
			std::unique_ptr<godot::RID[]> rids = std::make_unique<godot::RID[]>(count);

			m_item_offsets.fill_owned_buffer(rids.get());

			for (godot::RID* rid = rids.get(); rid < rids.get() + count; rid++)
			{
				T* item = m_item_offsets.get_or_null(*rid);

				if (item > target_item)
				{
					m_item_offsets.replace(*rid, item - 1);
				}
			}

			m_item_offsets.free(id);

			T* end = reinterpret_cast<T*>(m_buffer.ptrw() + m_buffer.size());

			std::copy(target_item, end, target_item - 1);
		}

		const godot::PackedFloat32Array& GetBuffer()
		{
			return m_buffer;
		}

	private:
		godot::RID_PtrOwner<T> m_item_offsets;
		godot::PackedFloat32Array m_buffer;
	};
}