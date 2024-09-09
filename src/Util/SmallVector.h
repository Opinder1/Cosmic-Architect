#pragma once

#include "Debug.h"

#include <cstddef>
#include <cstdint>

template<class DataT, class DerivedT>
class SmallVectorBase
{
public:
    using iterator = DataT*;
    using const_iterator = const DataT*;

public:
    SmallVectorBase() {}

    iterator begin()
    {
        return get_derived().ptr();
    }

    iterator end()
    {
        return get_derived().ptr() + m_size;
    }

    const_iterator begin() const
    {
        return get_derived().ptr();
    }

    const_iterator end() const
    {
        return get_derived().ptr() + m_size;
    }

    const_iterator cbegin()
    {
        return get_derived().ptr();
    }

    const_iterator cend()
    {
        return get_derived().ptr() + m_size;
    }

    DataT* data()
    {
        return get_derived().ptr();
    }

    const DataT* data() const
    {
        return get_derived().ptr();
    }

    iterator push_back(DataT item)
    {
        return emplace_back(item);
    }

    iterator push_front(DataT item)
    {
        return emplace_front(item);
    }

    void pop_back()
    {
        if (m_size > 0)
        {
            std::destroy_at(end() - 1);
            m_size--;
        }
    }

    template<class... Args>
    iterator emplace_back(Args&&... args)
    {
        return emplace(end(), std::forward<Args>(args)...);
    }

    template<class... Args>
    iterator emplace_front(Args&&... args)
    {
        return emplace(begin(), std::forward<Args>(args)...);
    }

    template<class... Args>
    iterator emplace(iterator pos, Args&&... args)
    {
        if (!get_derived().grow(m_size + 1))
        {
            return end();
        }

        std::move_backward(pos, end(), pos + 1);

        DataT* item = end();

        new (item) DataT(std::forward<Args>(args)...);
        m_size++;

        return item;
    }

    iterator insert(iterator pos, const DataT& value)
    {
        return emplace(pos, value);
    }

    iterator insert(iterator pos, DataT&& value)
    {
        return emplace(pos, std::move(value));
    }

    template<class InputIt>
    iterator insert(iterator pos, size_t count, const DataT& value)
    {
        if (!get_derived().grow(m_size + count))
        {
            return end();
        }

        std::move_backward(pos, end(), pos + count);

        for (size_t i = 0; i < count; i++)
        {
            new (pos + i) DataT(value);
        }

        m_size += count;

        return pos;
    }

    template<class InputIt>
    iterator insert(iterator pos, InputIt first, InputIt last)
    {
        ptrdiff_t count = std::distance(last, first);

        if (!get_derived().grow(m_size + count))
        {
            return end();
        }

        std::move_backward(pos, end(), pos + count);

        for (size_t i = 0; i < count; i++)
        {
            new (pos + i) DataT(first + i);
        }

        m_size += count;

        return pos;
    }

    DataT& at(size_t index)
    {
        return *(begin() + index);
    }

    const DataT& at(size_t index) const
    {
        return *(begin() + index);
    }

    DataT& operator[](size_t index)
    {
        return at(index);
    }

    const DataT& operator[](size_t index) const
    {
        return at(index);
    }

    iterator erase(const_iterator pos)
    {
        return erase(pos, pos + 1);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        if (first < begin() || last > end())
        {
            return end();
        }

        std::destroy(first, last);

        if (last < end())
        {
            std::move(last, cend(), const_cast<iterator>(first));
        }

        m_size -= std::distance(first, last);

        return end();
    }

    void clear()
    {
        for (DataT& item : *this)
        {
            std::destroy_at(&item);
        }

        m_size = 0;
    }

    void resize(size_t new_size)
    {
        get_derived().grow(new_size);

        if (new_size > m_size)
        {
            DataT* new_last = begin() + new_size;
            for (DataT* item = begin(); item != new_last; item++)
            {
                new (item) DataT();
            }

            m_size = new_size;
        }
    }

    DataT& front()
    {
        return *begin();
    }

    const DataT& front() const
    {
        return *begin();
    }

    DataT& back()
    {
        return *(end() - 1);
    }

    const DataT& back() const
    {
        return *(end() - 1);
    }

    size_t size() const
    {
        return m_size;
    }

    size_t length() const
    {
        return size();
    }

    bool is_empty() const
    {
        return m_size == 0;
    }

protected:
    void swap(SmallVectorBase& other)
    {
        std::swap(m_size, other.m_size);
    }

    void swap_array_items(SmallVectorBase& other)
    {
        size_t common_size = std::min(m_size, other.m_size);

        size_t i = 0;
        for (; i < common_size; i++)
        {
            std::swap(at(i), other.at(i));
        }

        if (m_size > other.m_size)
        {
            for (; i < m_size; i++)
            {
                other.at(i) = std::move(at(i));
            }
        }
        else
        {
            for (; i < other.m_size; i++)
            {
                at(i) = std::move(other.at(i));
            }
        }
    }

private:
    DerivedT& get_derived()
    {
        return static_cast<DerivedT&>(*this);
    }

    const DerivedT& get_derived() const
    {
        return static_cast<const DerivedT&>(*this);
    }

protected:
    size_t m_size = 0;
};

template<class DataT, size_t k_capacity>
class SmallVector : public SmallVectorBase<DataT, SmallVector<DataT, k_capacity>>
{
    friend class SmallVectorBase<DataT, SmallVector<DataT, k_capacity>>;

    using Base = SmallVectorBase<DataT, SmallVector<DataT, k_capacity>>;

public:
    SmallVector() {}

    size_t max_size()
    {
        return k_capacity;
    }

    void reserve(size_t size) {}

    size_t capacity()
    {
        return k_capacity;
    }

    void shrink_to_fit() {}

    void swap(SmallVector& other) noexcept
    {
        Base::swap_array_items(other);

        Base::swap(other);
    }

private:
    DataT* ptr()
    {
        return reinterpret_cast<DataT*>(m_storage);
    }

    const DataT* ptr() const
    {
        return reinterpret_cast<const DataT*>(m_storage);
    }

    bool grow(size_t new_size)
    {
        if (new_size > k_capacity)
        {
            DEBUG_PRINT_WARN("Tried to emplace too many items in a constant capacity vector");
            return false;
        }
        
        return true;
    }

private:
    alignas(alignof(DataT)) std::byte m_storage[k_capacity * sizeof(DataT)];
};

template<class DataT, size_t k_capacity>
class GrowingSmallVector : public SmallVectorBase<DataT, GrowingSmallVector<DataT, k_capacity>>
{
    friend class SmallVectorBase<DataT, GrowingSmallVector<DataT, k_capacity>>;

    using Base = SmallVectorBase<DataT, GrowingSmallVector<DataT, k_capacity>>;

public:
    GrowingSmallVector() : m_buffer(storage_ptr()), m_capacity(k_capacity) {}

    ~GrowingSmallVector()
    {
        clear();

        if (m_buffer != storage_ptr())
        {
            delete[] m_buffer;
        }
    }

    size_t max_size()
    {
        return SIZE_MAX;
    }

    void reserve(size_t new_capacity)
    {
        if (new_capacity <= m_capacity)
        {
            return;
        }

        std::byte* new_buffer = new std::byte[sizeof(DataT) * new_capacity];
        DataT* new_ptr = reinterpret_cast<DataT*>(new_buffer);

        if (m_buffer != storage_ptr())
        {
            for (DataT& item : *this)
            {
                *new_ptr++ = std::move(item);
            }

            delete[] m_buffer;
        }

        m_buffer = new_buffer;
        m_capacity = new_capacity;
    }

    size_t capacity()
    {
        return m_capacity;
    }

    void shrink_to_fit()
    {
        if (m_buffer == storage_ptr()) // Don't reallocate if we are using the array buffer
        {
            return;
        }

        std::byte* new_buffer;

        if (Base::m_size <= m_storage)
        {
            new_buffer = storage_ptr(); // Go back to the array buffer if it fits all our items
        }
        else
        {
            new_buffer = new std::byte[sizeof(DataT) * Base::m_size];
        }

        DataT* new_ptr = static_cast<DataT*>(new_buffer);

        for (DataT& item : *this)
        {
            *new_ptr++ = std::move(item);
        }

        delete[] m_buffer;

        m_buffer = new_buffer;
        m_capacity = Base::m_size;
    }

    void swap(GrowingSmallVector& other) noexcept
    {
        bool using_storage = m_buffer == storage_ptr();
        bool other_using_storage = other.m_buffer == other.storage_ptr();

        if (using_storage || other_using_storage)
        {
            Base::swap_array_items(other);

            std::byte* new_buffer = other_using_storage ? storage_ptr() : other.m_buffer;
            std::byte* other_new_buffer = using_storage ? other.storage_ptr() : m_buffer;

            m_buffer = new_buffer;
            other.m_buffer = other_new_buffer;
        }
        else
        {
            // Fast pointer swap since both are using heap allocated memory
            std::swap(m_buffer, other.m_buffer);
        }

        std::swap(m_capacity, other.m_capacity);
        Base::swap(other);
    }

private:
    std::byte* storage_ptr()
    {
        return m_storage;
    }

    DataT* ptr()
    {
        return reinterpret_cast<DataT*>(m_buffer);
    }

    const DataT* ptr() const
    {
        return reinterpret_cast<const DataT*>(m_buffer);
    }

    bool grow(size_t new_size)
    {
        if (new_size > m_capacity)
        {
            reserve(m_capacity * 2);
        }

        return true;
    }

private:
    std::byte* m_buffer;
    size_t m_capacity;
    alignas(alignof(DataT)) std::byte m_storage[k_capacity * sizeof(DataT)];
};