#pragma once

#include "Debug.h"

#include <cstddef>
#include <cstdint>

// Circular buffer interface that is implemented with a static and growing buffer. Has the same api as a std::vector
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
        for (DataT& item : *this)
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
        size_t new_last;
        size_t old_last = m_last;

        if (!get_derived().get_new_last(new_last))
        {
            return end();
        }

        m_last = new_last;
        new(get_derived().ptr() + old_last) DataT(std::forward<Args>(args)...);

        return iterator{ get_derived(), old_last };
    }

    template<class... Args>
    iterator emplace_front(Args&&... args)
    {
        size_t new_first;

        if (!get_derived().get_new_first(new_first))
        {
            return end();
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

protected:
    size_t m_first = 0;
    size_t m_last = 0;
};

// Constant size buffer that items can be pushed to the front and popped from behind
template<class DataT, size_t k_capacity>
class CircularBuffer : public CircularBufferBase<DataT, CircularBuffer<DataT, k_capacity>>
{
    friend class CircularBufferBase<DataT, CircularBuffer<DataT, k_capacity>>;

    using Base = CircularBufferBase<DataT, CircularBuffer<DataT, k_capacity>>;

public:
    CircularBuffer() {}

    CircularBuffer(CircularBuffer&& other)
    {
        Base::m_first = other.m_first;
        Base::m_last = other.m_last;

        for (size_t index = Base::m_first; index != Base::m_last;)
        {
            *(ptr() + index) = std::move(*(other.ptr() + index));

            index = (index + 1) % capacity();
        }

        other.m_first = 0;
        other.m_last = 0;
    }

    size_t capacity() const
    {
        return k_capacity;
    }

private:
    // Get index of the last item after adding to the back. Return false if we ran out of space
    bool get_new_last(size_t& new_last)
    {
        new_last = (Base::m_last + 1) % capacity();

        if (new_last == Base::m_first)
        {
            DEBUG_PRINT_WARN("Tried to emplace too many items in a constant capacity circular buffer");
            return false;
        }

        return true;
    }

    // Get index of the first item after adding to the front.  Return false if we ran out of space
    bool get_new_first(size_t& new_first)
    {
        new_first = (Base::m_first + capacity() - 1) % capacity();

        if (new_first == Base::m_first)
        {
            DEBUG_PRINT_WARN("Tried to emplace too many items in a constant capacity circular buffer");
            return false;
        }

        return true;
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

// Growing buffer that items can be pushed to the front and popped from behind
template<class DataT>
class GrowingCircularBuffer : public CircularBufferBase<DataT, GrowingCircularBuffer<DataT>>
{
    friend class CircularBufferBase<DataT, GrowingCircularBuffer<DataT>>;

    using Base = CircularBufferBase<DataT, GrowingCircularBuffer<DataT>>;

public:
    GrowingCircularBuffer() {}

    explicit GrowingCircularBuffer(size_t capacity)
    {
        reserve(capacity);
    }

    GrowingCircularBuffer(GrowingCircularBuffer&& other)
    {
        Base::m_first = other.m_first;
        Base::m_last = other.m_last;
        m_buffer = other.m_buffer;
        m_capacity = other.m_capacity;

        other.m_first = 0;
        other.m_last = 0;
        other.m_buffer = nullptr;
        other.m_capacity = 0;
    }

    size_t capacity() const
    {
        return m_capacity;
    }

    void clear()
    {
        Base::clear();

        if (m_buffer != nullptr)
        {
            delete[] m_buffer;
        }

        m_capacity = 0;
    }

    // Resize the buffer to a certain capacity and move the current data to that buffer
    void reserve(size_t new_capacity)
    {
        if (new_capacity < 4)
        {
            new_capacity = 4;
        }

        // Allocate a new buffer
        size_t old_size = Base::size();
        std::byte* new_buffer = new std::byte[sizeof(DataT) * new_capacity];
        DataT* new_ptr = (DataT*)new_buffer;

        // If we have an old buffer then copy from it and delete it
        if (m_buffer != nullptr)
        {
            for (DataT& item : *this)
            {
                *new_ptr++ = std::move(item);
            }

            delete[] m_buffer;
        }

        // Initialize our values
        Base::m_first = 0;
        Base::m_last = old_size;
        m_buffer = new_buffer;
        m_capacity = new_capacity;
    }

private:
    // Get index of the last item after adding to the back
    bool get_new_last(size_t& new_last)
    {
        if (Base::size() + 1 >= capacity())
        {
            grow();
        }

        new_last = (Base::m_last + 1) % capacity();

        return true;
    }

    // Get index of the first item after adding to the front
    bool get_new_first(size_t& new_first)
    {
        if (Base::size() + 1 >= capacity())
        {
            grow();
        }

        new_first = (Base::m_first + capacity() - 1) % capacity();

        return true;
    }

    // Grow our buffer to have more space
    void grow()
    {
        reserve(capacity() * 2);
    }

    DataT* ptr()
    {
        return (DataT*)m_buffer;
    }

    const DataT* ptr() const
    {
        return (const DataT*)m_buffer;
    }

private:
    std::byte* m_buffer = nullptr;
    size_t m_capacity = 0;
};