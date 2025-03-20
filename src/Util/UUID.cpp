#include "UUID.h"

bool UUID::operator==(const UUID& other) const
{
    return data[0] == other.data[0] && data[1] == other.data[1];
}

size_t UUIDHash::operator()(const UUID& uuid) const
{
    static_assert(sizeof(UUID) == 16);

    return uuid.data[0] ^ uuid.data[1];
}