#pragma once

#include "UUID.h"

#include "Message/Message.h"

namespace sim
{
	struct NewEntityComponent {};

	struct DeletedEntityComponent {};

	// A component to give the entity a global unique identifier
	struct UUIDComponent
	{
		UUID id;
	};

	// This component makes the entity own a simulation that it will manage
	struct SimulationComponent
	{
		UUID simulation_id;

		MessageQueue queue;
	};

	struct NewSimulationComponent {};
}