#include "MessageFactory.h"

namespace sim
{
	MessageFactoryBase::MessageFactoryBase(Message::Type type, uint32_t index) :
		m_type(type),
		m_index(index)
	{}

	MessageFactoryBase::~MessageFactoryBase()
	{}

	Message::Type MessageFactoryBase::GetType() const
	{
		return m_type;
	}

	uint32_t MessageFactoryBase::GetIndex() const
	{
		return m_index;
	}
}