#pragma once

#include "Simulation/UUID.h"

namespace sim
{
	// A source from which messages will be generated
	class MessageSender
	{
	public:
		explicit MessageSender(UUID id);
		~MessageSender();

		UUID GetUUID() const;

	private:
		UUID m_id;
	};
}