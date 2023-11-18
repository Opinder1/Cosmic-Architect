#pragma once

#include "Event.h"

#include <robin_hood/robin_hood.h>

namespace sim
{
	// Priority for observers recieving packets. For each priority it is still handled in order of observer addition
	enum class Priority
	{
		Lowest, // Lowest priorities when we dont want to handle if event was cancelled
		VeryLow,
		Low,
		Normal, // Center which should be used by most
		High,
		VeryHigh,
		Highest, // Higher prioritys should be when expecting to cancel the event
	};

	// Ordering for observers being registered. First means that the event will be placed before all of its same priority
	enum class Ordering
	{
		First,
		Last
	};

	// An event that can be queued to be processed later.
	// This class is designed to be stored in a contiguous array so we can only store events of a max size of 4
	class QueuedEvent
	{
	public:
		static constexpr size_t k_max_size = 32;

		QueuedEvent(QueuedEvent&&) = default;

		template<class EventT>
		explicit QueuedEvent(EventT&& event)
		{
			static_assert(sizeof(EventT) <= k_max_size, "Queued events can be max 4 bytes");
			static_assert(std::is_trivially_destructible_v<EventT>, "Queued events must be trivially destructible");

			Get<EventT>() = std::move(event);
			m_type = GetEventType<EventT>();
		}

		Event& GetEvent()
		{
			return *reinterpret_cast<Event*>(m_memory);
		}

		Event::Type GetType()
		{
			return m_type;
		}

	private:
		uint8_t m_memory[32];
		Event::Type m_type;
	};

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

		// A callback list that will be automatically sorted by priority on emplacement
		using CallbackList = std::vector<CallbackEntry>;

		// A list of messages that will be processed at a later point
		using EventQueue = std::vector<QueuedEvent>;

	public:
		explicit EventDispatcher();
		
		~EventDispatcher();

		// Post a Event of any type
		// Return Pass on success and Cancel if an observer tried to cancel the Event
		template<class EventT>
		void PostEvent(const EventT& event)
		{
			static_assert(std::is_base_of_v<Event, EventT>);

			return PostEventGeneric(static_cast<const Event&>(event), GetEventType<EventT>());
		}

		// Post a Event of any type
		// Return Pass on success and Cancel if an observer tried to cancel the Event
		template<class EventT>
		void PostQueuedEvent(EventT&& event)
		{
			static_assert(std::is_base_of_v<Event, EventT>);

			return PostQueuedEventGeneric(QueuedEvent{ std::move(event) });
		}
		
		// Subscribe to an event with a callback
		template<class EventT>
		void Subscribe(const EventCallback<EventT>& callback, Priority priority = Priority::Normal, Ordering ordering = Ordering::First)
		{
			SubscribeGeneric(GetGenericCallback(callback), GetEventType<EventT>(), priority, ordering);
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
		void PostEventGeneric(const Event& event, Event::Type event_type);

		void PostQueuedEventGeneric(QueuedEvent&& event);

		void SubscribeGeneric(const EventCallback<Event>& callback, Event::Type event_type, Priority priority, Ordering ordering);

		void UnsubscribeGeneric(const EventCallback<Event>& callback, Event::Type event_type);

	private:
		// Internal functions which use a retrieved list which would have been obtained using the type
		void PostEventInternal(CallbackList& list, const Event& event);

		void SubscribeInternal(CallbackList& list, const EventCallback<Event>& callback, Priority priority, Ordering ordering);

		void UnsubscribeInternal(CallbackList& list, const EventCallback<Event>& callback);

	private:
		robin_hood::unordered_node_map<Event::Type, CallbackList> m_callback_lists; // One list of callbacks for each event type

		EventQueue m_queue;
	};
}