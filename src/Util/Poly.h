#pragma once

#include "Debug.h"

#include <vector>

constexpr const uint16_t k_invalid_poly_offset = UINT16_MAX;

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

class Poly
{
	friend class PolyType;

public:
	Poly();
	Poly(std::byte* ptr);

	template<class T>
	const T& GetEntry(PolyEntry<T> entry) const
	{
		DEBUG_ASSERT(IsValid(), "The poly should be valid");
		DEBUG_ASSERT(entry.offset != k_invalid_poly_offset, "The entry should be invalid");

		return *reinterpret_cast<const T*>(ptr + entry.offset);
	}

	template<class T>
	T& GetEntry(PolyEntry<T> entry)
	{
		DEBUG_ASSERT(IsValid(), "The poly should be valid");
		DEBUG_ASSERT(entry.offset != k_invalid_poly_offset, "The entry should be invalid");

		return *reinterpret_cast<T*>(ptr + entry.offset);
	}

	bool IsValid() const;

	bool operator==(Poly other) const;
	bool operator!=(Poly other) const;

private:
	std::byte* ptr;
};

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

	template<class T>
	PolyEntry<T> AddEntry()
	{
		return PolyEntry<T>(AddEntry(Construct<T>, Destruct<T>, sizeof(T)));
	}

	uint16_t AddEntry(FactoryCB construct, FactoryCB destruct, uint16_t size);

	Poly CreatePoly();

	void DestroyPoly(Poly poly);

	uint16_t GetSize() const;

private:

private:
	std::vector<Entry> m_entries;
	uint16_t m_total_size = 0;
};