#pragma once

#include "Util/Nocopy.h"

#include <godot_cpp/variant/string.hpp>

#include <robin_hood/robin_hood.h>

#include <cstdint>
#include <random>

namespace sim
{
	// An id that should be unique regardless of the circumstances in which it was generated
	class UUID
	{
	public:
		static const UUID k_empty_uuid;

	public:
		// Generate a random uuid using thread safe global random. (Avoid using this often and prefer using local random)
		static UUID GenerateRandom();

		// Create empty uuid (value of 0)
		UUID();

		// Initialize with two 64 bit integers
		explicit UUID(uint64_t first, uint64_t second);

		// Initialize with a uuid in string form. If invalid then the uuid will have a value of 0
		explicit UUID(const std::string& string);
		explicit UUID(const godot::String& string);

		// Comparitor needed for maps and any other place where we want to compare
		bool operator==(const UUID& other) const;
		bool operator!=(const UUID& other) const;

		// Check if we are empty. Can also compare with k_empty_uuid
		bool IsEmpty() const;

		// Get the uuid as a string
		std::string ToString() const;

		// Get the uuid as a string
		godot::String ToGodotString() const;

		// Get a byte of the uuid. Any index higher than 8 is invalid
		unsigned char GetByte(uint8_t index) const;

		// Get the first 4 bytes of the uuid
		uint64_t GetFirst() const;

		// Get the second 4 bytes of the uuid
		uint64_t GetSecond() const;

	private:
		// Helper for loading a uuid from a string
		bool FromString(const std::string& string);

		// Helper for loading a uuid from a godot string
		bool FromString(const godot::String& string);

	private:
		union
		{
			unsigned char m_bytes[16];

			struct
			{
				uint64_t m_first;
				uint64_t m_second;
			};
		};
	};

	// A simple random number generate for non secure but fast random needs.
	// The main use of this is to have one in each thread to generate UUIDs
	class UUIDGenerator : nocopy
	{
	public:
		// Initialise with time based seed
		explicit UUIDGenerator();

		// Initialise with supplied seed
		explicit UUIDGenerator(uint64_t seed);

		// Get a new random number and step forward the seed.
		UUID Generate();

	private:
		std::mt19937_64 m_generator;
		std::uniform_int_distribution<uint64_t> m_distribution;
	};
}

// Hash so we can use it in maps
template<>
struct robin_hood::hash<sim::UUID>
{
	size_t operator()(const sim::UUID& uuid) const noexcept;
};