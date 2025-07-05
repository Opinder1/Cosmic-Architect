#include "UUID.h"

#include "Util/Debug.h"

#include <random>

namespace
{
	const char TOHEXCHAR[16] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };

    static std::random_device dev;

    thread_local std::mt19937_64 rng(dev());

	unsigned int HexChrToDec(char c)
	{
		return (unsigned char)((c) <= '9' ? ((c)-'0') : 10 + (c)-((c) <= 'F' ? 'A' : 'a'));
	}

	void UUIDToString(const unsigned char(&uuid_bytes)[16], char* buffer) // Buffer should be 36 in size
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
					DEBUG_PRINT_ERROR("UUID should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX");
					return false;
				}
			}
			else
			{
				if (!isxdigit(c))
				{
					DEBUG_PRINT_ERROR("UUID should be in the format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX");
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
}

UUID::UUID() :
	m_data{ 0, 0 }
{ }

UUID::UUID(uint64_t first, uint64_t second) :
	m_data{first, second}
{ }

UUID::UUID(const std::string& string) :
	m_data{0, 0}
{
	if (string.size() != 36)
	{
		DEBUG_PRINT_ERROR("String length for uuid should be 36");
	}
	else if (!UUIDFromString(m_bytes, string.c_str()))
	{
		DEBUG_PRINT_ERROR("Failed to load uuid from string");
	}
}

UUID::UUID(const godot::String& string) :
	m_data{ 0, 0 }
{
	if (string.length() != 36)
	{
		DEBUG_PRINT_ERROR("String length for uuid should be 36");
	}
	else if (!UUIDFromString(m_bytes, string.utf8().get_data()))
	{
		DEBUG_PRINT_ERROR("Failed to load uuid from string");
	}
}

bool UUID::operator==(const UUID& other) const
{
    return m_data[0] == other.m_data[0] && m_data[1] == other.m_data[1];
}

bool UUID::operator!=(const UUID& other) const
{
	return m_data[0] != other.m_data[0] && m_data[1] != other.m_data[1];
}

// Get the uuid as a string
std::string UUID::ToString() const
{
	char buffer[36];

	UUIDToString(m_bytes, buffer);

	return std::string(buffer, 36);
}

// Get the uuid as a string
godot::String UUID::ToGodotString() const
{
	char buffer[36];

	UUIDToString(m_bytes, buffer);

	return godot::String::utf8(buffer, 36);
}

UUID GenerateUUID()
{
    return UUID{ rng(), rng() };
}

namespace std
{
    size_t hash<UUID>::operator()(const UUID& uuid) const noexcept
    {
        static_assert(sizeof(UUID) == 16);

        return uuid.m_data[0] ^ uuid.m_data[1];
    }
}