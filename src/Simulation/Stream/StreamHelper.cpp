#include "StreamHelper.h"

#include "Simulation/UUID.h"

#include <godot_cpp/core/error_macros.hpp>

namespace sim
{
	namespace StreamHelper
	{
		bool IsBigEndian()
		{
			union {
				uint32_t i;
				char c[4];
			} b_int = { 0x01020304 };

			return b_int.c[0] == 1;
		}

		void SwapBinary16(uint16_t& value)
		{
			value = (value >> 8) | (value << 8);
		}

		void SwapBinary32(uint32_t& value)
		{
			std::uint32_t tmp = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
			value = (tmp << 16) | (tmp >> 16);
		}

		void SwapBinary64(uint64_t& value)
		{
			value = ((value & 0x00000000FFFFFFFFull) << 32) | ((value & 0xFFFFFFFF00000000ull) >> 32);
			value = ((value & 0x0000FFFF0000FFFFull) << 16) | ((value & 0xFFFF0000FFFF0000ull) >> 16);
			value = ((value & 0x00FF00FF00FF00FFull) << 8) | ((value & 0xFF00FF00FF00FF00ull) >> 8);
		}

		bool ReadSize(ByteStream& src, size_t& size, ByteStream::GroupMagnitude type)
		{
			switch (type)
			{
			case ByteStream::GroupMagnitude::Short:
				return ReadRaw<uint8_t>(src, (uint8_t*)&size, 1) == 1;

			case ByteStream::GroupMagnitude::Medium:
				return ReadRaw<uint16_t>(src, (uint16_t*)&size, 1) == 1;

			case ByteStream::GroupMagnitude::Long:
				return ReadRaw<uint32_t>(src, (uint32_t*)&size, 1) == 1;

			default:
				ERR_FAIL_V_MSG(false, "Invalid size magnitude");
			}
		}

		bool WriteSize(ByteStream& dest, size_t size, ByteStream::GroupMagnitude magnitude)
		{
			switch (magnitude)
			{
			case ByteStream::GroupMagnitude::Short:
				ERR_FAIL_COND_V_MSG(size > UINT8_MAX, false, "The size was larger than a short group can handle");

				return WriteRaw<uint8_t>(dest, (const uint8_t*)&size, 1) == 1;

			case ByteStream::GroupMagnitude::Medium:
				ERR_FAIL_COND_V_MSG(size > UINT16_MAX, false, "The size was larger than a medium group can handle");

				return WriteRaw<uint16_t>(dest, (const uint16_t*)&size, 1) == 1;

			case ByteStream::GroupMagnitude::Long:
				ERR_FAIL_COND_V_MSG(size > UINT32_MAX, false, "The size was larger than a long group can handle");

				return WriteRaw<uint32_t>(dest, (const uint32_t*)&size, 1) == 1;

			default:
				ERR_FAIL_V_MSG(false, "Invalid size magnitude");
			}
		}

		size_t ReadString(ByteStream& src, std::string& output, ByteStream::GroupMagnitude magnitude)
		{
			size_t size;
			ERR_FAIL_COND_V_MSG(!ReadSize(src, size, magnitude), 0, "Failed to read string size");

			ERR_FAIL_COND_V_MSG(size > (src.GetSize() - src.GetPos()), 0, "The string size stored goes out of the streams bounds");

			output.resize(size);

			return ReadRaw<char>(src, output.data(), output.size());
		}

		bool ReadUUID(ByteStream& src, UUID& id)
		{
			uint64_t first, second;

			ERR_FAIL_COND_V_MSG(Read<uint64_t>(src, first), false, "Failed to read first byte of uuid");
			ERR_FAIL_COND_V_MSG(Read<uint64_t>(src, second), false, "Failed to read second byte of uuid");

			id = UUID(first, second);

			return true;
		}

		bool WriteUUID(ByteStream& src, const UUID& id)
		{
			ERR_FAIL_COND_V_MSG(Write<uint64_t>(src, id.GetFirst()), false, "Failed to read first byte of uuid");
			ERR_FAIL_COND_V_MSG(Write<uint64_t>(src, id.GetSecond()), false, "Failed to read second byte of uuid");
			
			return true;
		}

		size_t WriteString(ByteStream& dest, const std::string& input, ByteStream::GroupMagnitude magnitude)
		{
			ERR_FAIL_COND_V_MSG(!WriteSize(dest, (uint32_t)input.size(), magnitude), 0, "Failed to write string size");

			return WriteRaw<char>(dest, input.data(), input.size());
		}

		size_t ReadString(ByteStream& src, char* output, uint32_t max_output, ByteStream::GroupMagnitude magnitude)
		{
			size_t size;
			ERR_FAIL_COND_V_MSG(!ReadSize(src, size, magnitude), 0, "Failed to read string size");

			ERR_FAIL_COND_V_MSG(size >= max_output, false, "The string size stored goes out of the requested max output size");
			ERR_FAIL_COND_V_MSG(size > (src.GetSize() - src.GetPos()), false, "The string size stored goes out of the streams bounds");

			size_t bytes_read = ReadRaw<char>(src, output, size);

			output[bytes_read] = '\0';

			return bytes_read;
		}

		size_t WriteString(ByteStream& dest, const char* input, uint32_t input_size, ByteStream::GroupMagnitude type)
		{
			ERR_FAIL_COND_V_MSG(!WriteSize(dest, input_size, type), 0, "Failed to write string data");
			
			return WriteRaw<char>(dest, input, input_size);
		}
	}
}