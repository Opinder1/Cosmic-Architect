#include "Poly.h"

#include "Debug.h"

Poly::Poly() :
	ptr(nullptr)
{}

Poly::Poly(std::byte* ptr) :
	ptr(ptr)
{}

bool Poly::IsValid() const
{
	return ptr != nullptr;
}

bool Poly::operator==(Poly other) const
{
	return ptr == other.ptr;
}

bool Poly::operator!=(Poly other) const
{
	return ptr != other.ptr;
}

uint16_t PolyType::AddEntry(FactoryCB construct, FactoryCB destruct, uint16_t size)
{
	DEBUG_ASSERT((uint32_t)m_total_size + size <= UINT16_MAX, "A poly can only support a size of up to 65535");

	Entry& entry = m_entries.emplace_back();

	entry.construct = construct;
	entry.destruct = destruct;
	entry.offset = m_total_size;

	m_total_size += size;

	return entry.offset;
}

Poly PolyType::CreatePoly()
{
	DEBUG_ASSERT(m_total_size > 0, "We should have entries for the poly type");

	std::byte* ptr = reinterpret_cast<std::byte*>(malloc(m_total_size));

	for (Entry& entry : m_entries)
	{
		entry.construct(ptr + entry.offset);
	}

	return Poly{ ptr };
}

void PolyType::DestroyPoly(Poly poly)
{
	DEBUG_ASSERT(poly.IsValid(), "A valid poly should be provided for destruction");

	for (Entry& entry : m_entries)
	{
		entry.destruct(poly.ptr + entry.offset);
	}

	free(poly.ptr);
}

uint16_t PolyType::GetSize() const
{
	return m_total_size;
}