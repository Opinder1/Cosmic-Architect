#include "Poly.h"

#include "Debug.h"

Poly::Poly() :
	m_ptr(nullptr)
{}

Poly::Poly(std::byte* ptr) :
	m_ptr(ptr)
{}

#if defined(POLY_DEBUG)
Poly::Poly(std::byte* ptr, const std::shared_ptr<void*>& type) :
	m_ptr(ptr),
	m_type(type)
{}
#endif

bool Poly::operator==(Poly other) const
{
	return m_ptr == other.m_ptr;
}

bool Poly::operator!=(Poly other) const
{
	return m_ptr != other.m_ptr;
}

bool Poly::IsValid() const
{
	return m_ptr != nullptr;
}

PolyType::PolyType()
{
#if defined(POLY_DEBUG)
	m_type = std::make_shared<void*>(this);
#endif
}

PolyType::~PolyType()
{
#if defined(POLY_DEBUG)
	DEBUG_ASSERT(m_num_poly == 0, "All poly objects created from this type should have been destroyed first");
#endif
}

uint16_t PolyType::AddEntry(FactoryCB construct, FactoryCB destruct, uint16_t size)
{
	DEBUG_ASSERT((uint32_t)m_total_size + size <= UINT16_MAX, "A poly can only support a size of up to 65535");

#if defined(POLY_DEBUG)
	DEBUG_ASSERT(m_num_poly == 0, "We should only be adding entries before we create any poly of this type");
#endif

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

#if defined(POLY_DEBUG)
	m_num_poly++;

	return Poly(ptr, m_type);
#else
	return Poly(ptr);
#endif
}

void PolyType::DestroyPoly(Poly poly)
{
	DEBUG_ASSERT(poly.IsValid(), "A valid poly should be provided for destruction");

#if defined(POLY_DEBUG)
	DEBUG_ASSERT(poly.m_type == m_type, "The poly should be created from this type");

	m_num_poly--;
#endif

	for (Entry& entry : m_entries)
	{
		entry.destruct(poly.m_ptr + entry.offset);
	}

	free(poly.m_ptr);
}

uint16_t PolyType::GetSize() const
{
	return m_total_size;
}