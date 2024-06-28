#pragma once

#include <cstddef>
#include <cstdint>

template<class DataT, size_t k_capacity, bool k_growing>
class SmallVector;

template<class DataT, size_t k_capacity>
class SmallVector<DataT, k_capacity, false>
{
public:
	SmallVector() {}

private:
	size_t m_size;
	std::byte m_storage[k_capacity * sizeof(DataT)];
};

template<class DataT, size_t k_capacity>
class SmallVector<DataT, k_capacity, true>
{
public:
	SmallVector() {}

private:
	size_t m_size;
	size_t m_capacity;
	DataT* m_buffer;
	std::byte m_storage[k_capacity * sizeof(DataT)];
};