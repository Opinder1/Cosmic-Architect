#pragma once

#include "Nocopy.h"

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

template<class CommandT>
class CommandSwapBuffer : Nocopy, Nomove
{
public:
	CommandSwapBuffer() {}

	void AddCommand(const CommandT& value)
	{
		m_write.emplace_back(value);
	}

	void AddCommand(CommandT&& value)
	{
		m_write.emplace_back(std::move(value));
	}

	// Write the changes to the exchange buffer
	void PublishCommands()
	{
		if (m_write.size() > 0 && !m_ready.load(std::memory_order_acquire))
		{
			m_read = std::move(m_write);

			m_ready.store(true, std::memory_order_release);
		}
	}

	// Obtain the latest changes made by the writer if there are any
	void RetrieveCommands(std::vector<CommandT>& out)
	{
		if (m_ready.load(std::memory_order_acquire))
		{
			out = std::move(m_read);

			m_ready.store(false, std::memory_order_release);
		}
	}

private:
	std::atomic_bool m_ready = false;
	alignas(k_cache_line) std::vector<CommandT> m_write;
	alignas(k_cache_line) std::vector<CommandT> m_read;
};

template<class T>
class alignas(k_cache_line) TripleBuffer : Nocopy, Nomove
{
public:
	TripleBuffer() {}

	TripleBuffer(const T& write, const T& swap, const T& read) :
		m_write(write),
		m_swap(swap),
		m_read(read)
	{}

	TripleBuffer(T&& write, T&& swap, T&& read) :
		m_write(std::move(write)),
		m_swap(std::move(swap)),
		m_read(std::move(read))
	{}

	// Write the changes to the exchange buffer
	void Publish()
	{
		if (!m_ready.load(std::memory_order_acquire))
		{
			std::swap(m_swap, m_write);

			m_ready.store(true, std::memory_order_release);
		}
	}

	// Obtain the latest changes made by the writer if there are any
	T& Retrieve()
	{
		if (m_ready.load(std::memory_order_acquire))
		{
			std::swap(m_read, m_swap);

			m_ready.store(false, std::memory_order_release);
		}

		return m_read;
	}

private:
	std::atomic_bool m_ready = false;
	T m_write;
	T m_swap;
	T m_read;
};