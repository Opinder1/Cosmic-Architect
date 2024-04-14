#pragma once

#include <atomic>
#include <utility>

template<class T>
class TripleBuffer
{
public:
	TripleBuffer() {}

	void EndWrite()
	{
		m_intermediate = std::move(m_write);
		m_flag.store(true, std::memory_order_release);
	}

	void StartRead()
	{
		if (m_flag.load(std::memory_order_acquire))
		{
			m_read = std::move(m_intermediate);
			m_flag.store(false, std::memory_order_release);
		}
	}

	T& Write()
	{
		return m_write;
	}

	const T& Read()
	{
		return m_read;
	}

private:
	std::atomic_bool m_flag;

	T m_write;
	T m_intermediate;
	T m_read;
};