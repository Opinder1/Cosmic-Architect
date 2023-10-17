#include "UUID.h"

#include "Time.h"

#include <mutex>
#include <random>

namespace sim
{
	const char TOHEXCHAR[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };

	unsigned int HexChrToDec(char c)
	{
		return (unsigned char)((c) <= '9' ? ((c)-'0') : 10 + (c)-((c) <= 'F' ? 'A' : 'a'));
	}

	void UUIDToString(const unsigned char (&uuid_bytes)[16], char* buffer) // Buffer should be 36 in size
	{
		char* dest = buffer;

		int src_byte = 0;

		for (int i = 0; i < 4; ++i)
		{
			*dest++ = TOHEXCHAR[(uuid_bytes[src_byte] >> 4) & 0xF];
			*dest++ = TOHEXCHAR[uuid_bytes[src_byte] & 0xF];
			++src_byte;
		}
		*dest++ = '-';

		for (int i = 0; i < 2; ++i)
		{
			*dest++ = TOHEXCHAR[(uuid_bytes[src_byte] >> 4) & 0xF];
			*dest++ = TOHEXCHAR[uuid_bytes[src_byte] & 0xF];
			++src_byte;
		}
		*dest++ = '-';

		for (int i = 0; i < 2; ++i)
		{
			*dest++ = TOHEXCHAR[(uuid_bytes[src_byte] >> 4) & 0xF];
			*dest++ = TOHEXCHAR[uuid_bytes[src_byte] & 0xF];
			++src_byte;
		}
		*dest++ = '-';

		for (int i = 0; i < 2; ++i)
		{
			*dest++ = TOHEXCHAR[(uuid_bytes[src_byte] >> 4) & 0xF];
			*dest++ = TOHEXCHAR[uuid_bytes[src_byte] & 0xF];
			++src_byte;
		}
		*dest++ = '-';

		for (int i = 0; i < 6; ++i)
		{
			*dest++ = TOHEXCHAR[(uuid_bytes[src_byte] >> 4) & 0xF];
			*dest++ = TOHEXCHAR[uuid_bytes[src_byte] & 0xF];
			++src_byte;
		}
	}

	bool UUIDFromString(unsigned char(&uuid_bytes)[16], const char* buffer) // Buffer should be 36 in size
	{
		char uuid_str[32];
		char* out = uuid_str;

		for (int i = 0; i < 36; ++i)
		{
			char c = buffer[i];
			if (i == 8 || i == 13 || i == 18 || i == 23)
			{
				if (c != '-')
				{
					return false;
				}
			}
			else
			{
				if (!isxdigit(c))
				{
					return false;
				}
				*out = (char)tolower(c);
				++out;
			}
		}

		for (int byte = 0; byte < 16; ++byte)
		{
			unsigned char v1 = HexChrToDec(uuid_str[byte * 2]);
			unsigned char v2 = HexChrToDec(uuid_str[byte * 2 + 1]);
			uuid_bytes[byte] = (unsigned char)((v1 << 4) | v2);
		}

		return true;
	}

	uint64_t TimeBasedUniqueInt()
	{
		// Time that can change based on operating system
		uint64_t sys = SystemClock::now().time_since_epoch().count();

		// Time that should not change in program execution
		uint64_t local = Clock::now().time_since_epoch().count();

		return sys * local;
	}

	Random::Random() :
		m_left_seed(TimeBasedUniqueInt()),
		m_right_seed(TimeBasedUniqueInt())
	{

	}

	Random::Random(uint64_t left_seed, uint64_t right_seed) :
		m_left_seed(left_seed),
		m_right_seed(right_seed)
	{

	}

	uint64_t Random::GenerateLeft()
	{
		m_left_seed = 6364136223846793005 * m_left_seed + 1;
		return m_left_seed;
	}

	uint64_t Random::GenerateRight()
	{
		m_right_seed = 6364136223846793005 * m_right_seed + 1;
		return m_right_seed;
	}

	const UUID UUID::k_empty_uuid;

	Random global_random;
	std::mutex global_random_mutex;

	UUID UUID::GenerateRandom()
	{
		std::lock_guard lock(global_random_mutex);

		return UUID(global_random);
	}

	UUID::UUID() :
		m_first(0),
		m_second(0)
	{

	}

	UUID::UUID(uint64_t m_first, uint64_t m_second) :
		m_first(m_first),
		m_second(m_second)
	{

	}

	UUID::UUID(Random& random) :
		m_first(random.GenerateLeft()),
		m_second(random.GenerateRight())
	{

	}

	UUID::UUID(const std::string& string)
	{
		if (!FromString(string))
		{
			m_first = 0;
			m_second = 0;
		}
	}

	UUID::UUID(const godot::String& string)
	{
		if (!FromString(string))
		{
			m_first = 0;
			m_second = 0;
		}
	}

	bool UUID::operator==(const UUID& other) const
	{
		return m_first == other.m_first && m_second == other.m_second;
	}

	bool UUID::operator!=(const UUID& other) const
	{
		return m_first != other.m_first || m_second != other.m_second;
	}

	bool UUID::IsEmpty() const
	{
		return m_first == 0 && m_second == 0;
	}

	std::string UUID::ToString() const
	{
		char buffer[36];

		UUIDToString(m_bytes, buffer);

		return std::string(buffer, 36);
	}

	godot::String UUID::ToGodotString() const
	{
		char buffer[36];

		UUIDToString(m_bytes, buffer);

		return godot::String::utf8(buffer, 36);
	}

	bool UUID::FromString(const std::string& string)
	{
		if (string.size() != 36)
		{
			return false;
		}

		return UUIDFromString(m_bytes, string.c_str());
	}

	bool UUID::FromString(const godot::String& string)
	{
		if (string.length() != 36)
		{
			return false;
		}

		return UUIDFromString(m_bytes, string.utf8().get_data());
	}

	unsigned char UUID::GetByte(uint8_t index) const
	{
		// Return the index modulo 8 as a safety in case the user requests a higher byte
		return m_bytes[index % 8];
	}

	uint64_t UUID::GetFirst() const
	{
		return m_first;
	}

	uint64_t UUID::GetSecond() const
	{
		return m_second;
	}
}

size_t robin_hood::hash<sim::UUID>::operator()(const sim::UUID& uuid) const noexcept
{
	//return robin_hood::hash<uint64_t>()(uuid.GetFirst()) ^ (robin_hood::hash<uint64_t>()(uuid.GetSecond()) << 1);

	return uuid.GetFirst() ^ uuid.GetSecond();
}