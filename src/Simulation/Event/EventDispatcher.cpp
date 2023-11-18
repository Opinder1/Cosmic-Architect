#include "EventDispatcher.h"

#include "Util/Debug.h"

#include <godot_cpp/variant/string.hpp>

namespace sim
{
	EventDispatcher::EventDispatcher()
	{

	}

	EventDispatcher::~EventDispatcher()
	{
#if DEBUG
		for (auto&& [type, callback_list] : m_callback_lists)
		{
			if (!callback_list.empty())
			{
				DEBUG_PRINT_ERROR("The callback list for the type '" + godot::String::num_uint64(type) + "' was not empty on destruction");
			}
		}
#endif
	}

	void EventDispatcher::ProcessEventQueue()
	{
		for (QueuedEvent& entry : m_queue)
		{
			PostEventGeneric(entry.GetEvent(), entry.GetType());
		}

		m_queue.clear();
	}

	void EventDispatcher::PostEventGeneric(const Event& event, Event::Type event_type)
	{
		auto it = m_callback_lists.find(event_type);

		if (it != m_callback_lists.end())
		{
			PostEventInternal(it->second, event);
		}
	}

	void EventDispatcher::PostQueuedEventGeneric(QueuedEvent&& event)
	{
		m_queue.emplace_back(std::move(event));
	}

	void EventDispatcher::SubscribeGeneric(const EventCallback<Event>& callback, Event::Type event_type, Priority priority, Ordering ordering)
	{
		SubscribeInternal(m_callback_lists[event_type], callback, priority, ordering);
	}

	void EventDispatcher::UnsubscribeGeneric(const EventCallback<Event>& callback, Event::Type event_type)
	{
		auto it = m_callback_lists.find(event_type);

		if (it == m_callback_lists.end())
		{
			DEBUG_PRINT_ERROR("Event type '" + godot::String::num_uint64(event_type) + "' has no observers so unsubscribe failed");
			return;
		}

		UnsubscribeInternal(it->second, callback);

		// Erase the callback list if it is empty
		if (it->second.size() == 0)
		{
			m_callback_lists.erase(it);
		}
	}

	void EventDispatcher::PostEventInternal(CallbackList& list, const Event& event)
	{
		for (CallbackEntry& callback : list) // The list should be sorted by priority
		{
			callback.callback(event);
		}
	}

	void EventDispatcher::SubscribeInternal(CallbackList& list, const EventCallback<Event>& callback, Priority priority, Ordering ordering)
	{
		CallbackList::iterator it;

		if (ordering == Ordering::Last)
		{
			// Here we emplace before the first item we find that is lower priority than this callback
			it = std::find_if(list.begin(), list.end(), [priority](CallbackEntry& other) { return other.priority < priority; });
		}
		else
		{
			// Here we emplace after the first item we find that is higher priority than this callback
			it = std::find_if(list.rbegin(), list.rend(), [priority](CallbackEntry& other) { return other.priority > priority; }).base();
		}

		// We emplace before the iterator which works even if the iterator is at the end
		list.emplace(it, CallbackEntry{ callback, priority });
	}

	void EventDispatcher::UnsubscribeInternal(CallbackList& list, const EventCallback<Event>& callback)
	{
		auto find_it = std::find_if(list.begin(), list.end(), [&](CallbackEntry& other) { return other.callback == callback; });

		if (find_it == list.end())
		{
			DEBUG_PRINT_ERROR("Event type does not have the observer");
			return;
		}

		list.erase(find_it);
	}
}