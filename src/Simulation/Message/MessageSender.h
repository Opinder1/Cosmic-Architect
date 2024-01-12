#pragma once

#include "Simulation/UUID.h"

namespace sim
{
	// A source from which messages will be generated
	class MessageSender
	{
	protected:
		explicit MessageSender(UUID id);
		~MessageSender();

	public:
		UUID GetUUID() const;

	private:
		UUID m_id;
	};
}