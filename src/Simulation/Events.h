#pragma once

#include "UUID.h"

#include "Message/Message.h"

#include "Util/Time.h"

namespace sim
{
	// Called when a thread messager has stopped execution. Should be sent internally
	struct MessagerStopEvent : Event
	{
		explicit MessagerStopEvent() {}
	};

	// Calls when another thread messager links. Should be sent internally
	struct MessagerLinkEvent : Event
	{
		explicit MessagerLinkEvent(UUID messager_id) :
			messager_id(messager_id)
		{}

		UUID messager_id;
	};

	// Calls when another thread messager unlinks. Should be sent internally
	struct MessagerUnlinkEvent : Event
	{
		explicit MessagerUnlinkEvent(UUID messager_id) :
			messager_id(messager_id)
		{}

		UUID messager_id;
	};

	// Called when a simulation starts running. Should be sent internally
	struct SimulationStartEvent : Event
	{
		explicit SimulationStartEvent() {}
	};

	// Called when a simulation is stopping execution. Should be sent internally
	struct SimulationRequestStopMessage : Message
	{
		explicit SimulationRequestStopMessage(const MessageSender& sender) : Message(sender) {}
		explicit SimulationRequestStopMessage() : Message(Unattested{}) {}
	};

	// Called when a thread has acquired the simulation and it will start to be manually ticked. Should be sent internally
	struct SimulationThreadAcquireMessage : Message
	{
		explicit SimulationThreadAcquireMessage(const MessageSender& sender) : Message(sender) {}
		explicit SimulationThreadAcquireMessage() : Message(Unattested{}) {}
	};

	// Called when a thread has released the simulation and it will no longer be manually ticked. Should be sent internally
	struct SimulationThreadReleaseMessage : Message
	{
		explicit SimulationThreadReleaseMessage(const MessageSender& sender) : Message(sender) {}
		explicit SimulationThreadReleaseMessage() : Message(Unattested{}) {}
	};

	// Called when a simulation has stopped execution. Should be sent internally
	struct SimulationStopEvent : Event
	{
		explicit SimulationStopEvent() {}
	};

	// Called at regular intervals by the simulation to progress. Should be sent internally
	struct SimulationTickEvent : Event
	{
		explicit SimulationTickEvent(Clock::time_point tick_start, Clock::duration timestep) :
			tick_start(tick_start),
			timestep(timestep)
		{}

		Clock::time_point tick_start;
		Clock::duration timestep;
	};

	// When we recieve this event we should process all entities with the new component
	struct ProcessNewEntitiesEvent : Event
	{
		explicit ProcessNewEntitiesEvent() {}
	};

	// When we recieve this event we should process all entities with the deleted component
	struct ProcessDeletedEntitiesEvent : Event
	{
		explicit ProcessDeletedEntitiesEvent() {}
	};

	// When we recieve this event we should process all entities with the parent deleted component
	struct ProcessDeletedParentsEvent : Event
	{
		explicit ProcessDeletedParentsEvent() {}
	};

	// Try to free any long unused resources or shrink to fit any arrays
	struct AttemptFreeMemoryMessage : Message
	{
		explicit AttemptFreeMemoryMessage() : Message(Unattested{}) {}
	};
}