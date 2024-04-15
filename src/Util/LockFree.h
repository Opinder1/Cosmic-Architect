#pragma once

#include <atomic>
#include <utility>

template<class T>
class LockFreeSwapBuffer
{
	enum class State : std::uint_fast8_t
	{
		ShouldRead,
		ShouldWrite,
	};

public:
	LockFreeSwapBuffer() {}

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

template<class T>
class LockFreeTripleBuffer
{
	static const std::uint_fast8_t k_ready_bit = 0b0100;
	static const std::uint_fast8_t k_index_mask = 0b0011;

public:
	LockFreeTripleBuffer() {}

	// Set our changes to a written buffer and get a new write buffer. The written buffer can then swap the changes with the read buffer.
	void ApplyWrite()
	{
		// Make sure to add the ready flag to the buffer we send
		std::uint_fast8_t ready_buffer = m_write_buffer | k_ready_bit;

		// Exchange our current write buffer with a new buffer.
		std::uint_fast8_t new_write_buffer = m_transfer_buffer.exchange(ready_buffer, std::memory_order_release);

		// Make sure to take off the mask for our write buffer in case it has it
		m_write_buffer = new_write_buffer & k_index_mask; 
	}

	// Get the latest written buffers changes to our read buffer.
	void ObtainRead()
	{
		std::uint_fast8_t ready_buffer = m_transfer_buffer.load(std::memory_order_acquire);

		// If the ready buffer has not been written to then don't try and swap the read
		if (!(ready_buffer & k_ready_bit))
		{
			return;
		}

		// Check if the transfer buffer is still the ready buffer and if so replace it with our read buffer
		if (m_transfer_buffer.compare_exchange_strong(ready_buffer, m_read_buffer, std::memory_order_release))
		{
			// If we succeed then set our read buffer to the ready buffer making sure to take of the mask
			m_read_buffer = ready_buffer & k_index_mask;
		}
	}

	T& Write()
	{
		return m_buffers[m_write_buffer];
	}

	const T& Read()
	{
		return m_buffers[m_read_buffer];
	}

private:
	T m_buffers[3] = {};
	std::uint_fast8_t m_write_buffer = 0;
	std::uint_fast8_t m_read_buffer = 1;
	std::atomic_uint_fast8_t m_transfer_buffer = 2;
};