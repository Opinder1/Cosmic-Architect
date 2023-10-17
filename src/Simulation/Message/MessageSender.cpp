#include "MessageSender.h"
#include "MessageRegistry.h"

namespace sim
{
	MessageSender::MessageSender(UUID id) :
		m_id(id)
	{
		MessageRegistry::GetSingleton()->RegisterMessageSender(*this);
	}

	MessageSender::~MessageSender()
	{
		MessageRegistry::GetSingleton()->UnregisterMessageSender(*this);
	}

	UUID MessageSender::GetUUID() const
	{
		return m_id;
	}
}