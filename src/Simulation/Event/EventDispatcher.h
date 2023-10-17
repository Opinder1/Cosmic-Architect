#pragma once

#include "Event.h"

#include <robin_hood/robin_hood.h>

namespace sim
{
	// An event dispatcher which procedures can register callbacks for different event types
	// Other procedures can then post events which will be handled by all registered callbacks for that type
	class EventDispatcher
	{
	private:
		// A single callback entry which is sortyed by priority
		struct CallbackEntry
		{
			EventCallback<Event> callback;
			Priority priority;
		};

		// A message that will be processed at a later point
		struct QueueEntry
		{
			EventPtr event;
			Event::Type type;
		};

		// A callback list that will be automatically sorted by priority on emplacement
		using CallbackList = std::vector<CallbackEntry>;

		// A list of messages that will be processed at a later point
		using EventQueue = std::vector<QueueEntry>;

	public:
		explicit EventDispatcher();
		
		~EventDispatcher();

		// Post a Event of any type
		// Return Pass on success and Cancel if an observer tried to cancel the Event
		template<class EventT>
		void PostEvent(const EventT& event)
		{
			static_assert(std::is_base_of_v<Event, EventT>);

			return PostEvent(static_cast<const Event&>(event), GetEventType<EventT>());
		}

		// Post a Event of any type
		// Return Pass on success and Cancel if an observer tried to cancel the Event
		template<class EventT>
		void PostQueuedEvent(EventT&& event)
		{
			static_assert(std::is_base_of_v<Event, EventT>);

			return PostQueuedEvent(std::make_unique<EventT>(std::move(event)), GetEventType<EventT>());
		}
		
		// Subscribe to an event with a callback
		template<class EventT>
		void Subscribe(const EventCallback<EventT>& callback, sim::Priority priority = sim::Priority::Normal)
		{
			SubscribeGeneric(GetGenericCallback(callback), GetEventType<EventT>(), priority);
		}

		// Unsubscribe from an event with a callback.
		// The callback must be the same function and object
		template<class EventT>
		void Unsubscribe(const EventCallback<EventT>& callback)
		{
			UnsubscribeGeneric(GetGenericCallback(callback), GetEventType<EventT>());
		}

	protected:
		// Process the messages in the queue
		void ProcessEventQueue();

		// Generic functions which use type variable instead of template
		void PostEvent(const Event& event, Event::Type event_type);

		void PostQueuedEvent(EventPtr&& event, Event::Type event_type);

		void SubscribeGeneric(const EventCallback<Event>& callback, Event::Type event_type, Priority priority);

		void UnsubscribeGeneric(const EventCallback<Event>& callback, Event::Type event_type);

	private:
		// Internal functions which use a retrieved list which would have been obtained using the type
		void PostEventInternal(CallbackList& list, const Event& event);

		void SubscribeInternal(CallbackList& list, const EventCallback<Event>& callback, Priority priority);

		void UnsubscribeInternal(CallbackList& list, const EventCallback<Event>& callback);

	private:
		robin_hood::unordered_node_map<Event::Type, CallbackList> m_callback_lists; // One list of callbacks for each event type

		EventQueue m_queue;
	};
}