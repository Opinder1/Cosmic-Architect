#pragma once

#include "UUID.h"

#include "Message/Message.h"

namespace sim
{
	// Assign this to any entity created in the current tick
	struct NewEntityComponent {};

	// Assign this to any entity deleted in the current tick
	struct DeletedEntityComponent {};

	// A component to give the entity a global unique identifier
	struct IDComponent
	{
		UUID id;
	};

	// This entity has a messager that we can send messages to
	struct LinkedMessagerComponent
	{
		UUID messager_id;
		MessageQueue queued_messages;
	};

	// This component notes the linked messager is a simulation in another thread
	struct LinkedThreadSimulationComponent {};

	// This component notes the linked messager is a simulation in another process
	struct LinkedNetworkSimulationComponent {};

	// This entity has an owner that is the authority over it
	struct OwnerComponent
	{
		UUID owner_simulation_id;
	};
}