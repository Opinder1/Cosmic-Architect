#pragma once

#include "Debug.h"
#include "Nocopy.h"

#include <vector>
#include <memory>

constexpr const uint16_t k_invalid_poly_offset = UINT16_MAX;

template<class T>
class PolyEntry;
class PolyType;

// An instance of a poly type
class Poly
{
	friend class PolyType;

public:
	Poly();
	Poly(std::byte* ptr);
#if defined(POLY_DEBUG)
	Poly(std::byte* ptr, const std::shared_ptr<void>& type);
#endif

	bool operator==(Poly other) const;
	bool operator!=(Poly other) const;

	// Check if this is an allocated poly or just an empty object
	bool IsValid() const;

	// Get a specific entry of this poly
	template<class T>
	const T& GetEntry(PolyEntry<T> entry) const
	{
		DEBUG_ASSERT(IsValid(), "The poly should be valid");
		DEBUG_ASSERT(entry.IsValid(), "The entry should be invalid");
#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_type == entry.m_type, "The poly entry should be from the same type");
#endif

		return *reinterpret_cast<const T*>(m_ptr + entry.m_offset);
	}

	// Get a specific entry of this poly
	template<class T>
	T& GetEntry(PolyEntry<T> entry)
	{
		DEBUG_ASSERT(IsValid(), "The poly should be valid");
		DEBUG_ASSERT(entry.IsValid(), "The entry should be invalid");
#if defined(POLY_DEBUG)
		DEBUG_ASSERT(m_type == entry.m_type, "The poly entry should be from the same type");
#endif

		return *reinterpret_cast<T*>(m_ptr + entry.m_offset);
	}

private:
	std::byte* m_ptr;

#if defined(POLY_DEBUG)
	std::shared_ptr<void> m_type = nullptr;
#endif
};

// A type entry of a poly type that every poly created will have
template<class T>
class PolyEntry
{
	friend class Poly;

public:
	PolyEntry() : m_offset(k_invalid_poly_offset) {}
	PolyEntry(uint16_t offset) : m_offset(offset) {}
#if defined(POLY_DEBUG)
	PolyEntry(uint16_t offset, const std::shared_ptr<void>& type) : m_offset(offset), m_type(type) {}
#endif

	bool IsValid() const
	{
		return m_offset != k_invalid_poly_offset;
	}

private:
	uint16_t m_offset;

#if defined(POLY_DEBUG)
	std::shared_ptr<void> m_type = nullptr;
#endif
};

// A system for creating runtime defined structs which are efficently allocated in memory
class PolyType : Nocopy
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
	PolyType();
	~PolyType();

	PolyType(PolyType&&) = default;
	PolyType& operator=(PolyType&&) = default;

	// Add a type entry that all polys created from this type will have
	template<class T>
	PolyEntry<T> AddEntry()
	{
		uint16_t offset = AddEntry(Construct<T>, Destruct<T>, sizeof(T));

#if defined(POLY_DEBUG)
		return PolyEntry<T>(offset, m_type);
#else
		return PolyEntry<T>(offset);
#endif
	}

	// Create a poly of this type
	Poly CreatePoly();

	// Destroy a poly of this type
	void DestroyPoly(Poly poly);

	// Get the size of the whole poly type
	uint16_t GetSize() const;

private:
	// Add an untyped entry with just a size and functions to initialize and uninitialize it
	uint16_t AddEntry(FactoryCB construct, FactoryCB destruct, uint16_t size);

private:
	std::vector<Entry> m_entries;
	uint16_t m_total_size = 0;

#if defined(POLY_DEBUG)
	size_t m_num_poly = 0;
	std::shared_ptr<void> m_type = nullptr;
#endif
};