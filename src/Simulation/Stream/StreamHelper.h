#pragma once

#include "ByteStream.h"

#include <string>
#include <vector>

namespace sim
{
	class UUID;

	namespace StreamHelper
	{
		bool IsBigEndian();

		void SwapBinary16(uint16_t& value);
		void SwapBinary32(uint32_t& value);
		void SwapBinary64(uint64_t& value);

		template<class T>
		void SwapBinary(T& value)
		{
			if constexpr (sizeof(T) == 1)
			{
				// Do nothing
			}
			else if constexpr (sizeof(T) == 2)
			{
				SwapBinary16(static_cast<uint16_t&>(value));
			}
			else if constexpr (sizeof(T) == 4)
			{
				SwapBinary32(static_cast<uint32_t&>(value));
			}
			else if constexpr (sizeof(T) == 8)
			{
				SwapBinary64(static_cast<uint64_t&>(value));
			}
			else
			{
				static_assert(false);
			}
		}

		template<class T>
		size_t ReadRaw(ByteStream& src, T* output, size_t count)
		{
			static_assert(std::is_arithmetic_v<T>);
			static_assert(sizeof(T) <= 8);

			size_t read = src.Read(output, count * sizeof(T));

			if (IsBigEndian())
			{
				for (size_t i = 0; i < count; i++)
				{
					SwapBinary<T>(output[i]);
				}
			}

			return read;
		}

		template<class T>
		size_t WriteRaw(ByteStream& dest, const T* input, size_t count)
		{
			static_assert(std::is_arithmetic_v<T>);
			static_assert(sizeof(T) <= 8);

			if (IsBigEndian() && sizeof(T) > 1)
			{
				for (size_t i = 0; i < count; i++)
				{
					T buffer = input[i];
					SwapBinary<T>(buffer);
					if (dest.Write(&buffer, sizeof(T)) != sizeof(T))
					{
						return i;
					}
				}

				return count;
			}
			else
			{
				return dest.Write(input, count * sizeof(T));
			}
		}

		template<class T>
		bool Read(ByteStream& src, T& output)
		{
			return ReadRaw<T>(src, &output, 1) == 1;
		}

		template<class T>
		bool Write(ByteStream& dest, const T& input)
		{
			return WriteRaw<T>(dest, &input, 1) == 1;
		}

		bool ReadSize(ByteStream& src, size_t& size, ByteStream::GroupMagnitude magnitude);
		bool WriteSize(ByteStream& dest, size_t size, ByteStream::GroupMagnitude magnitude);

		bool ReadUUID(ByteStream& src, UUID& id);
		bool WriteUUID(ByteStream& src, const UUID& id);

		size_t ReadString(ByteStream& src, std::string& output, ByteStream::GroupMagnitude magnitude);
		size_t WriteString(ByteStream& dest, const std::string& input, ByteStream::GroupMagnitude magnitude);

		size_t ReadString(ByteStream& src, char* output, uint32_t max_output, ByteStream::GroupMagnitude magnitude);
		size_t WriteString(ByteStream& dest, const char* input, uint32_t input_size, ByteStream::GroupMagnitude magnitude);

		template<class EnumT>
		bool ReadEnum(ByteStream& src, EnumT& output)
		{
			using Type = typename std::underlying_type<EnumT>::type;

			return Read<Type>(src, reinterpret_cast<Type&>(output));
		}

		template<class EnumT>
		bool WriteEnum(ByteStream& dest, EnumT input)
		{
			using Type = typename std::underlying_type<EnumT>::type;

			return Write<Type>(dest, static_cast<Type>(input));
		}

		template<class T>
		size_t ReadRawVector(ByteStream& src, std::vector<T>& output, ByteStream::GroupMagnitude magnitude)
		{
			size_t size;
			if (!ReadSize(src, size, magnitude))
			{
				return 0;
			}

			output.resize(size);

			return ReadRaw<T>(src, output.data(), size);
		}

		template<class T>
		size_t WriteRawVector(ByteStream& dest, const std::vector<T>& input, ByteStream::GroupMagnitude magnitude)
		{
			if (!WriteSize(dest, input.size(), magnitude))
			{
				return 0;
			}

			return WriteRaw<T>(dest, input.data(), input.size());
		}

		template<template<class> class S, class V>
		size_t ReadArray(ByteStream& src, S<V>& output, ByteStream::GroupMagnitude magnitude)
		{
			size_t size;
			if (!ReadSize(src, size, magnitude))
			{
				return 0;
			}

			for (size_t i = 0; i < size; i++)
			{
				V value;

				if (!Read<V>(src, value))
				{
					return i;
				}

				output.emplace(value);
			}

			return size;
		}

		template<template<class> class S, class V>
		size_t WriteArray(ByteStream& dest, const S<V>& input, ByteStream::GroupMagnitude magnitude)
		{
			if (!WriteSize(dest, input.size(), magnitude))
			{
				return 0;
			}

			size_t size = input.size();

			for (size_t i = 0; i < size; i++)
			{
				if (!Write<V>(dest, input[i]))
				{
					return i;
				}
			}

			return size;
		}

		template<template<class, class> class M, class K, class V>
		size_t ReadMap(ByteStream& src, M<K, V>& output, ByteStream::GroupMagnitude magnitude)
		{
			size_t size;
			if (!ReadSize(src, size, magnitude))
			{
				return 0;
			}

			for (size_t i = 0; i < size; i++)
			{
				K key;
				V value;

				if (!Read<K>(src, key) || !Read<V>(src, value))
				{
					return i;
				}

				output[key] = value;
			}

			return size;
		}

		template<template<class, class> class M, class K, class V>
		size_t WriteMap(ByteStream& dest, const M<K, V>& input, ByteStream::GroupMagnitude magnitude)
		{
			if (!WriteSize(dest, input.size(), magnitude))
			{
				return 0;
			}

			size_t size = input.size();

			auto it = input.begin();

			for (size_t i = 0; i < size; i++)
			{
				if (!Write<K>(dest, it->first) || !Write<V>(dest, it->second))
				{
					return i;
				}

				it++;
			}

			return size;
		}
	}
}