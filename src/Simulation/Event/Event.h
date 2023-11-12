#pragma once

#include "Util/Callback.h"

#include <memory>
#include <cstdint>
#include <typeinfo>

namespace sim
{
	// An event that is immediately sent to its observers. Should only be sent from a simulation to itself. It can have any data members as they will be safely accessed by the simulations thread
	struct Event
	{
		using Type = size_t; // The events C++ class type hash. This should not be serialised and instead the message index
		explicit Event() {};
	};

	// Get a unqiue type for each c++ event type
	template<class EventT>
	Event::Type GetEventType()
	{
		static_assert(std::is_base_of_v<Event, EventT>);

		return typeid(EventT).hash_code();
	}

	// Return a pointer to the specialised event type if the type matches
	template<class EventT>
	const EventT* GetEvent(const Event& event, Event::Type type)
	{
		if (type == GetEventType<EventT>())
		{
			return &static_cast<const EventT&>(event);
		}
		else
		{
			return nullptr;
		}
	}
	
	// Run the callback on the event if the event type matches
	template<class EventT, class Callback>
	void IfEvent(const Event& event, Event::Type type, Callback callback)
	{
		if (type == GetEventType<EventT>())
		{
			callback(static_cast<const EventT&>(event));
		}
	}

	// A callback for those who want to listen to an event
	template<class EventT>
	using EventCallback = cb::Callback<void(const EventT&)>;

	// Turn a callback into a generic callback for the base event type
	template<class EventT>
	const EventCallback<Event>& GetGenericCallback(const EventCallback<EventT>& callback)
	{
		return reinterpret_cast<const EventCallback<Event>&>(callback);
	}
}