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

	// All processing that should be done before TickEvent
	struct PreTickEvent : Event
	{
		explicit PreTickEvent() {}
	};

	// All processing that doesn't need ordering can listen to this
	struct TickEvent : Event
	{
		explicit TickEvent() {}
	};

	// All processing that should be done after TickEvent and requires TickEvent to be done
	struct PostTickEvent : Event
	{
		explicit PostTickEvent() {}
	};

	struct ProcessNewEntitiesEvent : Event
	{
		explicit ProcessNewEntitiesEvent() {}
	};
	
	struct ProcessDeletedEntitiesEvent : Event
	{
		explicit ProcessDeletedEntitiesEvent() {}
	};

	// Try to free any long unused resources or shrink to fit any arrays
	struct AttemptFreeMemoryEvent : Event
	{
		explicit AttemptFreeMemoryEvent() {}
	};
}