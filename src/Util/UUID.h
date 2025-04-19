#pragma once

#include <cstdint>
#include <type_traits>

// An id type that is large enough that we should never reasonably randomly generate two
// identical uuids in all runs of the program

struct UUID
{
    uint64_t data[2];

    bool operator==(const UUID& other) const;
};

UUID GenerateUUID();

namespace std
{
    template<>
    struct hash<UUID>
    {
        size_t operator()(const UUID& uuid) const noexcept;
    };
}