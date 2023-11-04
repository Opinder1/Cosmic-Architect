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
	struct UUIDComponent
	{
		UUID id;
	};

	// This entity has a messager that we can send messages to
	struct LinkedMessagerComponent
	{
		UUID id;

		MessageQueue queued_messages;
	};

	// This component notes the linked messager is a simulation
	struct LinkedSimulationComponent {};

	// The lifetime of the simulation is managed by this entity
	struct OwnedSimulationComponent {};

	// This component notes the linked messager is a remote simulation
	struct LinkedRemoteSimulationComponent {};

	// The lifetime of the remote simulation is managed by this entity
	struct OwnedRemoteSimulationComponent {};
}