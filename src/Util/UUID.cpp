#include "UUID.h"

bool UUID::operator==(const UUID& other) const
{
    return data[0] == other.data[0] && data[1] == other.data[1];
}

namespace std
{
    size_t hash<UUID>::operator()(const UUID& uuid) const noexcept
    {
        static_assert(sizeof(UUID) == 16);

        return uuid.data[0] ^ uuid.data[1];
    }
}