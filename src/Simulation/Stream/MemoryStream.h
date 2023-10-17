#pragma once

#include "ByteStream.h"

namespace sim
{
	class MemoryStreamBase : public ByteStream
	{
	public:
		MemoryStreamBase(char* data, size_t size);

		size_t GetPos() const final;

		void SetPos(size_t pos) final;

		void MovePos(int32_t offset) final;

		void SetEnd() final;

		size_t Read(void* buffer, size_t count) final;

		void Flush() final;

		bool CanExtend() const final;

		size_t GetSize() final;

	protected:
		// Get the underlying data pointed to. No null termination guarantee
		char* GetData() const;

		size_t GetStreamPos() const;

		void ForwardStreamPos(size_t offset);

	private:
		char* m_data;
		size_t m_size;
		size_t m_stream_pos;
	};

	class MemoryStream : public MemoryStreamBase
	{
	public:
		MemoryStream(char* data, size_t size);

		size_t Write(const void* buffer, size_t count) final;

		bool CanWrite() const final;

		// Get the underlying data pointed to. No null termination guarantee
		char* GetData() const;
	};

	class ConstMemoryStream : public MemoryStreamBase
	{
	public:
		ConstMemoryStream(const char* data, size_t size);

		size_t Write(const void* buffer, size_t count) final;

		bool CanWrite() const final;

		// Get the underlying data pointed to. No null termination guarantee
		const char* GetData() const;
	};
}