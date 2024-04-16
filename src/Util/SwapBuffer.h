#pragma once

#include <TKRZW/tkrzw_thread_util.h>

#include <shared_mutex>
#include <atomic>
#include <utility>

template<class T>
class SwapBuffer
{
public:
	SwapBuffer() {}

	// Write the changes to the exchange buffer
	void ApplyWrite()
	{
		if (m_exchange_mutex.try_lock())
		{
			m_exchange = std::move(m_write);

			m_ready.store(true, std::memory_order_release);
			m_exchange_mutex.unlock();
		}
	}

	// Get the write buffer. Should only be written to by one buffer
	T& Write()
	{
		return m_write;
	}

	// Obtain the latest changes made by the writer if there are any
	void Read(T& out)
	{
		std::lock_guard lock(m_exchange_mutex);

		if (m_ready.load(std::memory_order_acquire))
		{
			out = std::move(m_exchange);

			m_ready.store(false, std::memory_order_release);
		}
	}

	// Obtain the latest changes made by the writer if there are any
	bool TryRead(T& out)
	{
		if (m_exchange_mutex.try_lock())
		{
			if (m_ready.load(std::memory_order_acquire))
			{
				out = std::move(m_exchange);

				m_ready.store(false, std::memory_order_release);
				m_exchange_mutex.unlock();

				return true;
			}
		}

		return false;
	}

private:
	T m_write{};
	T m_exchange{};
	T m_read{};

	std::atomic_bool m_ready = false;

	tkrzw::SpinMutex m_exchange_mutex;
};