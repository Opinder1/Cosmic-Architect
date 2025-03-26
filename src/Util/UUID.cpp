#include "UUID.h"

#include <random>

namespace
{
    static std::random_device dev;

    thread_local std::mt19937_64 rng(dev());
}

bool UUID::operator==(const UUID& other) const
{
    return data[0] == other.data[0] && data[1] == other.data[1];
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

        return uuid.data[0] ^ uuid.data[1];
    }
}