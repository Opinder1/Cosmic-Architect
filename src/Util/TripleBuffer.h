#pragma once

#include <atomic>
#include <utility>

template<class T>
class TripleBuffer
{
	enum class State
	{
		ShouldRead,
		ShouldWrite,
	};

public:
	TripleBuffer() {}

	void ApplyWrite()
	{
		if (m_state.load(std::memory_order_acquire) == State::ShouldWrite)
		{
			m_exchange = std::move(m_write);
			m_state.store(State::ShouldRead, std::memory_order_release);
		}
	}

	void ObtainRead()
	{
		if (m_state.load(std::memory_order_acquire) == State::ShouldRead)
		{
			m_read = std::move(m_exchange);
			m_state.store(State::ShouldWrite, std::memory_order_release);
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
	T m_read{};
	T m_exchange{};
	T m_write{};

	std::atomic<State> m_state = State::ShouldWrite;
};