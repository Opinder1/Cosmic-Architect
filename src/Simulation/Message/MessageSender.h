#pragma once

#include "Simulation/UUID.h"

namespace sim
{
	class MessageRegistry;

	// A source from which messages will be generated
	class MessageSender
	{
	protected:
		explicit MessageSender(MessageRegistry& registry, UUID id);
		~MessageSender();

	public:
		MessageRegistry& GetMessageRegistry() const;

		UUID GetUUID() const;

	private:
		MessageRegistry& m_registry;
		UUID m_id;
	};
}