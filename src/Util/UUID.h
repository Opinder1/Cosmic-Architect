#pragma once

#include <cstdint>
#include <type_traits>

struct UUID
{
    uint64_t data[2];

    bool operator==(const UUID& other) const;
};

namespace std
{
    template<>
    struct hash<UUID>
    {
        size_t operator()(const UUID& uuid) const noexcept;
    };
}