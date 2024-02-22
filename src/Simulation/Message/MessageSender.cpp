#include "MessageSender.h"
#include "MessageRegistry.h"

namespace sim
{
	MessageSender::MessageSender(MessageRegistry& registry, UUID id) :
		m_registry(registry),
		m_id(id)
	{
		m_registry.RegisterMessageSender(*this);
	}

	MessageSender::~MessageSender()
	{
		m_registry.UnregisterMessageSender(*this);
	}

	MessageRegistry& MessageSender::GetMessageRegistry() const
	{
		return m_registry;
	}

	UUID MessageSender::GetUUID() const
	{
		return m_id;
	}
}