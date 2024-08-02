#pragma once

#include "Debug.h"

#include <cstdint>
#include <array>
#include <vector>

constexpr const size_t DynamicSpan = SIZE_MAX;

template<class T, size_t S>
class Span
{
public:
	Span() :
		m_data(nullptr)
	{}

	Span(T(&array)[S]) :
		m_data(array)
	{}

	Span(std::array<T, S>& array) :
		m_data(array.data())
	{}

	Span(const std::array<T, S>& array) :
		m_data(array.data())
	{}

	T* Data() const
	{
		return m_data;
	}

	size_t Size() const
	{
		return S;
	}

	T& operator[](size_t index) const
	{
		DEBUG_ASSERT(index <= S, "Trying to get an item out of range");

		return m_data[index];
	}

	bool Empty() const
	{
		return false;
	}

	T* First() const
	{
		return Data();
	}

	T* Last() const
	{
		return Data() + Size();
	}

	template<size_t Offset, size_t Count>
	Span<T, Count> SubSpan() const
	{
		static_assert(Offset + Count <= Size(), "Trying to get a subspan out of range");
		
		return Span<T, Count>(m_data + Offset);
	}

	template<size_t Count>
	Span<T, Count> SubSpan(size_t offset) const
	{
		DEBUG_ASSERT(offset + Count <= Size(), "Trying to get a subspan out of range");

		return Span<T, Count>(m_data + offset);
	}

	Span<T, DynamicSpan> SubSpan(size_t offset, size_t count) const
	{
		DEBUG_ASSERT(offset + count <= Size(), "Trying to get a subspan out of range");

		return Span<T, DynamicSpan>(m_data + offset, count);
	}

private:
	T* m_data;
};

template<class T>
class Span<T, DynamicSpan>
{
public:
	Span() :
		m_data(nullptr),
		m_size(0)
	{}

	Span(T* data, size_t size) :
		m_data(data),
		m_size(size)
	{}

	template<class It>
	Span(It* first, It* last) :
		m_data(first),
		m_size(last - first)
	{}

	template<size_t S>
	Span(T(&array)[S]) :
		m_data(array),
		m_size(S)
	{}

	template<size_t S>
	Span(std::array<T, S>& array) :
		m_data(array.data()),
		m_size(S)
	{}

	template<size_t S>
	Span(const std::array<T, S>& array) :
		m_data(array.data()),
		m_size(S)
	{}

	template<size_t S>
	Span(const Span<T, S>& span) :
		m_data(span.m_data),
		m_size(S)
	{}

	Span(std::vector<T>& vector) :
		m_data(vector.data()),
		m_size(vector.size())
	{}

	Span(const std::vector<T>& vector) :
		m_data(vector.data()),
		m_size(vector.size())
	{}

	T* Data() const
	{
		return m_data;
	}

	size_t Size() const
	{
		return m_size;
	}

	T& operator[](size_t index) const
	{
		DEBUG_ASSERT(index <= m_size, "Trying to get an item out of range");

		return m_data[index];
	}

	bool Empty() const
	{
		return m_size != 0;
	}

	T* First() const
	{
		return Data();
	}

	T* Last() const
	{
		return Data() + Size();
	}

	template<size_t Offset, size_t Count>
	Span<T, Count> SubSpan() const
	{
		DEBUG_ASSERT(Offset + Count <= Size(), "Trying to get a subspan out of range");

		return Span<T, Count>(m_data + Offset);
	}

	template<size_t Count>
	Span<T, Count> SubSpan(size_t offset) const
	{
		DEBUG_ASSERT(offset + Count <= Size(), "Trying to get a subspan out of range");

		return Span<T, Count>(m_data + offset);
	}

	Span<T, DynamicSpan> SubSpan(size_t offset, size_t count) const
	{
		DEBUG_ASSERT(offset + count <= Size(), "Trying to get a subspan out of range");

		return Span<T, DynamicSpan>(m_data + offset, count);
	}

private:
	T* m_data;
	size_t m_size;
};