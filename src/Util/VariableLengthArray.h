#pragma once

#include "Nocopy.h"

#include <cstdint>
#include <malloc.h>

template<class T>
class VariableLengthArray : Nocopy, Nomove
{
public:
	VariableLengthArray(void* data, size_t size) :
		m_data(static_cast<T*>(data)),
		m_size(size)
	{
		for (size_t i = 0; i < m_size; i++)
		{
			new(m_data + i) T();
		}
	}

	~VariableLengthArray()
	{
		std::destroy_n(m_data, m_size);
	}

	T* data()
	{
		return m_data;
	}

	T* begin()
	{
		return m_data;
	}

	T* end()
	{
		return m_data + m_size;
	}

	T& operator[](size_t index)
	{
		return m_data[index];
	}

	size_t size()
	{
		return m_size;
	}

private:
	T* m_data;
	uint32_t m_size;
};

#define MakeVariableLengthArray(T, S) VariableLengthArray<T>(alloca(S * sizeof(T)), S)