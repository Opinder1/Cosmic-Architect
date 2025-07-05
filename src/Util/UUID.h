#pragma once

#include <godot_cpp/variant/string.hpp>

#include <string>
#include <cstdint>
#include <type_traits>

// An id type that is large enough that we should never reasonably randomly generate two
// identical uuids in all runs of the program

struct UUID
{
    UUID();

    // Initialize with two 64 bit integers
    explicit UUID(uint64_t first, uint64_t second);

    // Initialize with a uuid in string form. If invalid then the uuid will have a value of 0
    explicit UUID(const std::string& string);
    explicit UUID(const godot::String& string);

    bool operator==(const UUID& other) const;
    bool operator!=(const UUID& other) const;

    // Get the uuid as a string
    std::string ToString() const;

    // Get the uuid as a string
    godot::String ToGodotString() const;

    union
    {
        unsigned char m_bytes[16];
        uint64_t m_data[2];
    };
};

UUID GenerateUUID();

void UUIDToString(UUID id, char* buffer);
bool UUIDFromString(UUID& id, const char* buffer);

namespace std
{
    template<>
    struct hash<UUID>
    {
        size_t operator()(const UUID& uuid) const noexcept;
    };
}