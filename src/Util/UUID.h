#pragma once

#include <cstdint>

struct UUID
{
    uint64_t data[2];

    bool operator==(const UUID& other) const;
};

struct UUIDHash
{
    size_t operator()(const UUID&) const;
};