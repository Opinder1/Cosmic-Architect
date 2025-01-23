#pragma once

#include "Nocopy.h"
#include "Debug.h"

#include <array>
#include <vector>
#include <thread>
#include <atomic>

constexpr static const size_t k_cache_line = std::hardware_destructive_interference_size;

constexpr static const size_t k_worker_thread_max = 16;

template<class DataT>
struct alignas(k_cache_line) AlignedData : DataT {};

// An array to store data for worker threads that avoids false sharing
template<class DataT>
struct PerThread : std::array<AlignedData<DataT>, k_worker_thread_max>, Nocopy, Nomove
{
	using std::array<AlignedData<DataT>, k_worker_thread_max>::array;
};

// A buffer where one thread writes and the other retrieves the whole item while being lock free
template<class T>
class SwapBuffer : Nocopy, Nomove
{
public:
	SwapBuffer() {}

	void Reset(const T& value)
	{
		m_value = value;
		m_ready.store(false, std::memory_order_release);
	}

	// Write the changes to the exchange buffer
	bool Publish(T& value)
	{
		if (!m_ready.load(std::memory_order_acquire))
		{
			std::swap(value, m_value);

			m_ready.store(true, std::memory_order_release);
			return true;
		}
		else
		{
			return false;
		}
	}

	// Obtain the latest changes made by the writer if there are any
	bool Retrieve(T& value)
	{
		if (m_ready.load(std::memory_order_acquire))
		{
			std::swap(value, m_value);

			m_ready.store(false, std::memory_order_release);
			return true;
		}
		else
		{
			return false;
		}
	}

private:
	T m_value;
	std::atomic_bool m_ready = false;
};