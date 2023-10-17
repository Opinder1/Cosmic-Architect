#pragma once

#include "Message/Message.h"

#include "UUID.h"
#include "Time.h"

namespace sim
{
	struct Message;

	// Called when a thread messager has stopped execution
	struct MessagerStopEvent : Event
	{
		explicit MessagerStopEvent() {}
	};

	// Calls when another thread messager links
	struct MessagerLinkEvent : Event
	{
		explicit MessagerLinkEvent(UUID messager_id) :
			messager_id(messager_id)
		{}

		UUID messager_id;
	};

	// Calls when another thread messager unlinks
	struct MessagerUnlinkEvent : Event
	{
		explicit MessagerUnlinkEvent(UUID messager_id) :
			messager_id(messager_id)
		{}

		UUID messager_id;
	};

	// Called when a simulation starts running
	struct SimulationStartEvent : Event
	{
		explicit SimulationStartEvent() {}
	};

	// Called when a simulation has stopped execution
	struct SimulationStopEvent : Event
	{
		explicit SimulationStopEvent() {}
	};

	// Called at regular intervals by the simulation to progress
	struct SimulationTickEvent : Event
	{
		explicit SimulationTickEvent(Clock::time_point tick_start, Clock::duration timestep) :
			tick_start(tick_start),
			timestep(timestep)
		{}

		Clock::time_point tick_start;
		Clock::duration timestep;
	};

	// Try to free any long unused resources or shrink to fit any arrays
	struct AttemptFreeMemoryEvent : Event
	{
		explicit AttemptFreeMemoryEvent() {}
	};

	struct ProcessMessageQueueEvent : Event
	{
		explicit ProcessMessageQueueEvent() {}
	};
}