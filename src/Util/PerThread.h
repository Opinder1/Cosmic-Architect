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

#if defined(DEBUG_ENABLED)
	void SetWriterThread(std::thread::id writer_id)
	{
		m_writer_id = writer_id;
	}
#endif

	T& GetWrite()
	{
		DEBUG_ASSERT(m_writer_id == std::this_thread::get_id(), "GetWrite() should be called by the writer thread");

		return m_write;
	}

	// Write the changes to the exchange buffer
	void Publish()
	{
		DEBUG_ASSERT(m_writer_id == std::this_thread::get_id(), "Publish() should be called by the writer thread");

		if (!m_ready.load(std::memory_order_acquire))
		{
			m_read = std::move(m_write);

			m_ready.store(true, std::memory_order_release);
		}
	}

	// Obtain the latest changes made by the writer if there are any
	void Retrieve(T& out)
	{
		if (m_ready.load(std::memory_order_acquire))
		{
			out = std::move(m_read);

			m_ready.store(false, std::memory_order_release);
		}
	}

private:
	std::atomic_bool m_ready = false;
	alignas(k_cache_line) T m_write;
	alignas(k_cache_line) T m_read;

#if defined(DEBUG_ENABLED)
	std::thread::id m_writer_id; // The thread that does updates and calls AddCommand() and PublishCommands() on it
#endif
};

// A triple buffer where a writer thread can write and a reader thread can read simultaneously while being lock free.
template<class T>
class alignas(k_cache_line) TripleBuffer : Nocopy, Nomove
{
public:
	TripleBuffer() {}

#if defined(DEBUG_ENABLED)
	void SetThreads(std::thread::id reader_id, std::thread::id writer_id)
	{
		m_reader_id = reader_id;
		m_writer_id = writer_id;
	}
#endif

	void Reset(const T& write, const T& swap, const T& read)
	{
		DEBUG_ASSERT(m_reader_id == std::thread::id{} && m_writer_id == std::thread::id{}, "Threads should not be set when resetting");

		m_write = write;
		m_swap = swap;
		m_read = read;
	}

	void Reset(T&& write, T&& swap, T&& read)
	{
		DEBUG_ASSERT(m_reader_id == std::thread::id{} && m_writer_id == std::thread::id{}, "Threads should not be set when resetting");

		m_write = std::move(write);
		m_swap = std::move(swap);
		m_read = std::move(read);
	}

	// Write changes the write buffer. Calling Publish() with references still around is undefined
	T& Write()
	{
		DEBUG_ASSERT(m_writer_id == std::this_thread::get_id(), "Write() should be called by the writer thread");

		return m_write;
	}

	// Write the changes to the exchange buffer
	void Publish()
	{
		DEBUG_ASSERT(m_writer_id == std::this_thread::get_id(), "Publish() should be called by the writer thread");

		if (!m_ready.load(std::memory_order_acquire))
		{
			std::swap(m_swap, m_write);

			m_ready.store(true, std::memory_order_release);
		}
	}

	// Check if the writer has made any changes
	bool Published()
	{
		return m_ready.load(std::memory_order_acquire);
	}

	// Obtain the latest changes made by the writer if there are any
	T& Retrieve()
	{
		DEBUG_ASSERT(m_reader_id == std::this_thread::get_id(), "Retrieve() should be called by the owner thread");

		if (m_ready.load(std::memory_order_acquire))
		{
			std::swap(m_read, m_swap);

			m_ready.store(false, std::memory_order_release);
		}

		return m_read;
	}

private:
	std::atomic_bool m_ready = false;
	alignas(k_cache_line) T m_write;
	alignas(k_cache_line) T m_swap;
	alignas(k_cache_line) T m_read;

#if defined(DEBUG_ENABLED)
	std::thread::id m_reader_id; // The thread that reads the cache and should call Retrieve() on it
	std::thread::id m_writer_id; // The thread that does updates and calls Write() and Publish() on it
#endif
};