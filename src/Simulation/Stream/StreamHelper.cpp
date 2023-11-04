#include "StreamHelper.h"

#include "Simulation/UUID.h"

#include "Util/Debug.h"

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
				if (ReadRaw<uint8_t>(src, (uint8_t*)&size, 1) != 1)
				{
					DEBUG_PRINT_ERROR("Failed to read short size");
					return false;
				}

				return true;

			case ByteStream::GroupMagnitude::Medium:
				if (ReadRaw<uint16_t>(src, (uint16_t*)&size, 1) != 1)
				{
					DEBUG_PRINT_ERROR("Failed to read medium size");
					return false;
				}

				return true;

			case ByteStream::GroupMagnitude::Long:
				if (ReadRaw<uint32_t>(src, (uint32_t*)&size, 1) != 1)
				{
					DEBUG_PRINT_ERROR("Failed to read long size");
					return false;
				}

				return true;

			default:
				DEBUG_PRINT_ERROR("Invalid size magnitude");
				return false;
			}
		}

		bool WriteSize(ByteStream& dest, size_t size, ByteStream::GroupMagnitude magnitude)
		{
			switch (magnitude)
			{
			case ByteStream::GroupMagnitude::Short:
				if (size > UINT8_MAX)
				{
					DEBUG_PRINT_ERROR("The size was larger than a short group can handle");
					return false;
				}

				if (WriteRaw<uint8_t>(dest, (const uint8_t*)&size, 1) != 1)
				{
					DEBUG_PRINT_ERROR("Failed to write short size");
					return false;
				}

				return true;

			case ByteStream::GroupMagnitude::Medium:
				if (size > UINT16_MAX)
				{
					DEBUG_PRINT_ERROR("The size was larger than a medium group can handle");
					return false;
				}

				if (WriteRaw<uint16_t>(dest, (const uint16_t*)&size, 1) != 1)
				{
					DEBUG_PRINT_ERROR("Failed to write medium size");
					return false;
				}

				return true;

			case ByteStream::GroupMagnitude::Long:
				if (size > UINT32_MAX)
				{
					DEBUG_PRINT_ERROR("The size was larger than a long group can handle");
					return false;
				}

				if (WriteRaw<uint32_t>(dest, (const uint32_t*)&size, 1) != 1)
				{
					DEBUG_PRINT_ERROR("Failed to write long size");
					return false;
				}

				return true;

			default:
				DEBUG_PRINT_ERROR("Invalid size magnitude");
				return false;
			}
		}

		bool ReadUUID(ByteStream& src, UUID& id)
		{
			uint64_t first, second;

			if (!Read<uint64_t>(src, first))
			{
				DEBUG_PRINT_ERROR("Failed to read first byte of uuid");
				return false;
			}

			if (!Read<uint64_t>(src, second))
			{
				DEBUG_PRINT_ERROR("Failed to read second byte of uuid");
				return false;
			}

			id = UUID(first, second);

			return true;
		}

		bool WriteUUID(ByteStream& src, const UUID& id)
		{
			if (!Write<uint64_t>(src, id.GetFirst()))
			{
				DEBUG_PRINT_ERROR("Failed to read first byte of uuid");
				return false;
			}

			if (!Write<uint64_t>(src, id.GetSecond()))
			{
				DEBUG_PRINT_ERROR("Failed to read second byte of uuid");
				return false;
			}
			
			return true;
		}

		size_t ReadString(ByteStream& src, std::string& output, ByteStream::GroupMagnitude magnitude)
		{
			size_t size;
			if (!ReadSize(src, size, magnitude))
			{
				DEBUG_PRINT_ERROR("Failed to read string size");
				return 0;
			}

			if (size > (src.GetSize() - src.GetPos()))
			{
				DEBUG_PRINT_ERROR("The string size stored goes out of the streams bounds");
				return 0;
			}

			output.resize(size);

			size_t bytes_read = ReadRaw<char>(src, output.data(), output.size());

			if (bytes_read != size)
			{
				DEBUG_PRINT_WARN("Failed to read whole of string data");
				output.resize(bytes_read);
			}

			return bytes_read;
		}

		size_t WriteString(ByteStream& dest, const std::string& input, ByteStream::GroupMagnitude magnitude)
		{
			if (!WriteSize(dest, (uint32_t)input.size(), magnitude))
			{
				DEBUG_PRINT_ERROR("Failed to write string size");
				return 0;
			}

			size_t bytes_written = WriteRaw<char>(dest, input.data(), input.size());

			if (bytes_written != input.size())
			{
				DEBUG_PRINT_WARN("Failed to write string data");
			}

			return bytes_written;
		}

		size_t ReadString(ByteStream& src, char* output, uint32_t max_output, ByteStream::GroupMagnitude magnitude)
		{
			size_t size;

			if (!ReadSize(src, size, magnitude))
			{
				DEBUG_PRINT_ERROR("Failed to read string size");
				return 0;
			}

			if (size >= max_output)
			{
				DEBUG_PRINT_ERROR("The string size stored goes out of the requested max output size");
				return 0;
			}

			if (size > (src.GetSize() - src.GetPos()))
			{
				DEBUG_PRINT_ERROR("The string size stored goes out of the streams bounds");
				return 0;
			}

			size_t bytes_read = ReadRaw<char>(src, output, size);

			if (bytes_read != size)
			{
				DEBUG_PRINT_WARN("Failed to read whole of string data");
			}

			output[bytes_read] = '\0';

			return bytes_read;
		}

		size_t WriteString(ByteStream& dest, const char* input, uint32_t input_size, ByteStream::GroupMagnitude type)
		{
			if (!WriteSize(dest, input_size, type))
			{
				DEBUG_PRINT_ERROR("Failed to write string data");
				return 0;
			}
			
			size_t bytes_written = WriteRaw<char>(dest, input, input_size);

			if (bytes_written != input_size)
			{
				DEBUG_PRINT_WARN("Failed to write string data");
			}

			return bytes_written;
		}
	}
}