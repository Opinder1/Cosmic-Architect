#pragma once

#include "Debug.h"

#include <cstddef>
#include <cstdint>

template<class DataT, class DerivedT>
class CircularBufferBase
{
    class Iterator
    {
    public:
        Iterator(DerivedT& buffer, size_t index) :
            m_buffer(buffer),
            m_index(index)
        {}

        DataT& operator*()
        {
            return *(m_buffer.ptr() + m_index);
        }

        void operator++()
        {
            m_index = (m_index + 1) % m_buffer.capacity();
        }

        bool operator!=(const Iterator& other) const
        {
            return m_index != other.m_index;
        }

    private:
        DerivedT& m_buffer;
        size_t m_index;
    };

    class ConstIterator
    {
    public:
        ConstIterator(const DerivedT& buffer, size_t index) :
            m_buffer(buffer),
            m_index(index)
        {}

        const DataT& operator*() const
        {
            return *(m_buffer.ptr() + m_index);
        }

        void operator++()
        {
            m_index = (m_index + 1) % m_buffer.capacity();
        }

        bool operator!=(const ConstIterator& other) const
        {
            return m_index != other.m_index;
        }

    private:
        const DerivedT& m_buffer;
        size_t m_index;
    };

public:
    using iterator = Iterator;
    using const_iterator = ConstIterator;

public:
    CircularBufferBase() {}

    ~CircularBufferBase()
    {
        for (DataT& item : get_derived())
        {
            std::destroy_at(&item);
        }
    }

    iterator begin()
    {
        return iterator{ get_derived(), m_first };
    }

    iterator end()
    {
        return iterator{ get_derived(), m_last };
    }

    const_iterator begin() const
    {
        return const_iterator{ get_derived(), m_first };
    }

    const_iterator end() const
    {
        return const_iterator{ get_derived(), m_last };
    }

    iterator push_back(DataT item)
    {
        return emplace_back(item);
    }

    iterator push_front(DataT item)
    {
        return emplace_front(item);
    }

    template<class... Args>
    iterator emplace_back(Args&&... args)
    {
        size_t new_last = (m_last + 1) % get_derived().capacity();

        if (new_last == m_first)
        {
            if (!get_derived().reallocate())
            {
                DEBUG_PRINT_WARN("Tried to emplace too many items in a constant capacity circular buffer");
                return end();
            }
        }

        size_t old_last = m_last;

        m_last = new_last;
        new(get_derived().ptr() + old_last) DataT(std::forward<Args>(args)...);

        return iterator{ get_derived(), old_last };
    }

    template<class... Args>
    iterator emplace_front(Args&&... args)
    {
        size_t new_first = (m_first + get_derived().capacity() - 1) % get_derived().capacity();

        if (new_first == m_last)
        {
            if (!get_derived().reallocate())
            {
                DEBUG_PRINT_WARN("Tried to emplace too many items in a constant capacity circular buffer");
                return end();
            }
        }

        m_first = new_first;
        new(get_derived().ptr() + m_first) DataT(std::forward<Args>(args)...);

        return iterator{ get_derived(), m_first };
    }

    void pop_back()
    {
        if (m_last == m_first)
        {
            return;
        }

        m_last = (m_last + get_derived().capacity() - 1) % get_derived().capacity();
        std::destroy_at(get_derived().ptr() + m_last);
    }

    void pop_front()
    {
        if (m_last == m_first)
        {
            return;
        }

        std::destroy_at(get_derived().ptr() + m_first);
        m_first = (m_first + 1) % get_derived().capacity();
    }

    DataT& at(size_t index)
    {
        size_t storage_index = (m_first + index) % get_derived().capacity();
        return *(get_derived().ptr() + storage_index);
    }

    const DataT& at(size_t index) const
    {
        size_t storage_index = (m_first + index) % get_derived().capacity();
        return *(get_derived().ptr() + storage_index);
    }

    DataT& operator[](size_t index)
    {
        return at(index);
    }

    const DataT& operator[](size_t index) const
    {
        return at(index);
    }

    DataT& front()
    {
        return *(get_derived().ptr() + m_first);
    }

    const DataT& front() const
    {
        return *(get_derived().ptr() + m_first);
    }

    DataT& back()
    {
        return *(get_derived().ptr() + m_last);
    }

    const DataT& back() const
    {
        return *(get_derived().ptr() + m_last);
    }

    void clear()
    {
        for (DataT& item : *this)
        {
            std::destroy_at(&item);
        }

        m_first = 0;
        m_last = 0;
    }

    size_t size() const
    {
        if (m_first > m_last)
        {
            return get_derived().capacity() - (m_first - m_last);
        }
        else
        {
            return m_last - m_first;
        }
    }

    size_t length() const
    {
        return size();
    }

    bool is_empty() const
    {
        return m_first == m_last;
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

private:
    size_t m_first = 0;
    size_t m_last = 0;
};

constexpr const size_t GrowingBuffer = SIZE_MAX;

template<class DataT, size_t k_capacity>
class CircularBuffer : public CircularBufferBase<DataT, CircularBuffer<DataT, k_capacity>>
{
    friend class CircularBufferBase<DataT, CircularBuffer<DataT, k_capacity>>;

    using Base = CircularBufferBase<DataT, CircularBuffer<DataT, k_capacity>>;

public:
    CircularBuffer() {}

    CircularBuffer(CircularBuffer&& other)
    {
        size_t items = other.length();

        Base::m_first = 0;
        Base::m_last = items;
        other.m_first = 0;
        other.m_last = 0;

        for (size_t i = 0; i < items; i++)
        {
            *(ptr() + i) = std::move(*(other.ptr() + i));
        }
    }

    size_t capacity() const
    {
        return k_capacity;
    }

private:
    bool reallocate()
    {
        return false;
    }

    DataT* ptr()
    {
        return (DataT*)&m_storage;
    }

    const DataT* ptr() const
    {
        return (const DataT*)&m_storage;
    }

private:
    std::byte m_storage[k_capacity * sizeof(DataT)];
};

template<class DataT>
class CircularBuffer<DataT, GrowingBuffer>
{
public:
    CircularBuffer() {}

private:
    size_t m_first = 0;
    size_t m_last = 0;
    DataT* m_buffer = nullptr;
    size_t m_capacity = 0;
};