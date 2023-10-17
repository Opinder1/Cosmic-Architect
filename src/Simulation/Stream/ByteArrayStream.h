#pragma once

#include "ByteStream.h"

#include <godot_cpp/variant/packed_byte_array.hpp>

namespace sim
{
	class ByteArrayStream : public ByteStream
	{
	public:
		ByteArrayStream();
		ByteArrayStream(const godot::PackedByteArray& data);

		size_t GetPos() const final;

		void SetPos(size_t pos) final;

		void MovePos(int32_t offset) final;

		void SetEnd() final;

		size_t Read(void* buffer, size_t count) final;

		size_t Write(const void* buffer, size_t count) final;

		void Flush() final;

		bool CanExtend() const final;

		bool CanWrite() const final;

		size_t GetSize() final;

		// Replace the data of the stream
		void SetData(const godot::PackedByteArray& data);

		// Get the underlying vector of this stream
		godot::PackedByteArray GetData() const;

	private:
		godot::PackedByteArray m_data;
		size_t m_stream_pos;
	};
}