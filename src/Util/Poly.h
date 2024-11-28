#pragma once

#include "Debug.h"

#include <vector>

constexpr const uint16_t k_invalid_poly_offset = UINT16_MAX;

// A type entry of a poly type that every poly created will have
template<class T>
class PolyEntry
{
	friend class Poly;

public:
	PolyEntry() : offset(k_invalid_poly_offset) {}
	PolyEntry(uint16_t offset) : offset(offset) {}

	bool IsValid() const
	{
		return offset != k_invalid_poly_offset;
	}

private:
	uint16_t offset;
};

// An instance of a poly type
class Poly
{
	friend class PolyType;

public:
	Poly();
	Poly(std::byte* ptr);

	bool operator==(Poly other) const;
	bool operator!=(Poly other) const;

	// Check if this is an allocated poly or just an empty object
	bool IsValid() const;

	// Get a specific entry of this poly
	template<class T>
	const T& GetEntry(PolyEntry<T> entry) const
	{
		DEBUG_ASSERT(IsValid(), "The poly should be valid");
		DEBUG_ASSERT(entry.offset != k_invalid_poly_offset, "The entry should be invalid");

		return *reinterpret_cast<const T*>(ptr + entry.offset);
	}

	// Get a specific entry of this poly
	template<class T>
	T& GetEntry(PolyEntry<T> entry)
	{
		DEBUG_ASSERT(IsValid(), "The poly should be valid");
		DEBUG_ASSERT(entry.offset != k_invalid_poly_offset, "The entry should be invalid");

		return *reinterpret_cast<T*>(ptr + entry.offset);
	}

private:
	std::byte* ptr;
};

// A system for creating runtime defined structs which are efficently allocated in memory
class PolyType
{
private:
	using FactoryCB = void (*)(std::byte*);

	struct Entry
	{
		FactoryCB construct = nullptr;
		FactoryCB destruct = nullptr;
		uint16_t offset = 0;
	};

private:
	template<class T>
	static void Construct(std::byte* ptr)
	{
		new (reinterpret_cast<T*>(ptr)) T();
	}

	template<class T>
	static void Destruct(std::byte* ptr)
	{
		std::destroy_at(reinterpret_cast<T*>(ptr));
	}

public:
	PolyType() {}

	// Add a type entry that all polys created from this type will have
	template<class T>
	PolyEntry<T> AddEntry()
	{
		return PolyEntry<T>(AddEntry(Construct<T>, Destruct<T>, sizeof(T)));
	}

	// Add an untyped entry with just a size and functions to initialize and uninitialize it
	uint16_t AddEntry(FactoryCB construct, FactoryCB destruct, uint16_t size);

	// Create a poly of this type
	Poly CreatePoly();

	// Destroy a poly of this type
	void DestroyPoly(Poly poly);

	// Get the size of the whole poly type
	uint16_t GetSize() const;

private:
	std::vector<Entry> m_entries;
	uint16_t m_total_size = 0;
};