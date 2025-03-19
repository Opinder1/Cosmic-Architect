#include "UUID.h"

size_t UUIDHash::operator()(const UUID& uuid) const
{
    static_assert(sizeof(UUID) == sizeof(uint64_t) * 2);

    uint64_t* arr = (uint64_t*)&uuid;

    return arr[0] ^ arr[1];
}