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
			if (!callback_list.entries.empty())
			{
				DEBUG_PRINT_ERROR(godot::vformat("The callback list for the type %s was not empty on destruction", callback_list.debug_name));
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

#if DEBUG
	void EventDispatcher::SubscribeGeneric(const EventCallback<Event>& callback, Event::Type event_type, Priority priority, Ordering ordering, const char* debug_name)
	{
		SubscribeInternal(m_callback_lists[event_type], callback, priority, ordering);

		m_callback_lists[event_type].debug_name = debug_name;
	}
#endif

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
		if (it->second.entries.size() == 0)
		{
			m_callback_lists.erase(it);
		}
	}

	void EventDispatcher::PostEventInternal(CallbackList& list, const Event& event)
	{
		for (CallbackEntry& callback : list.entries) // The list should be sorted by priority
		{
			callback.callback(event);
		}
	}

	void EventDispatcher::SubscribeInternal(CallbackList& list, const EventCallback<Event>& callback, Priority priority, Ordering ordering)
	{
		std::vector<CallbackEntry>::iterator it;

		if (ordering == Ordering::Last)
		{
			// Here we emplace before the first item we find that is lower priority than this callback
			it = std::find_if(list.entries.begin(), list.entries.end(), [priority](CallbackEntry& other) { return other.priority < priority; });
		}
		else
		{
			// Here we emplace after the first item we find that is higher priority than this callback
			it = std::find_if(list.entries.rbegin(), list.entries.rend(), [priority](CallbackEntry& other) { return other.priority > priority; }).base();
		}

		// We emplace before the iterator which works even if the iterator is at the end
		list.entries.emplace(it, CallbackEntry{ callback, priority });
	}

	void EventDispatcher::UnsubscribeInternal(CallbackList& list, const EventCallback<Event>& callback)
	{
		auto find_it = std::find_if(list.entries.begin(), list.entries.end(), [&](CallbackEntry& other) { return other.callback == callback; });

		if (find_it == list.entries.end())
		{
			DEBUG_PRINT_ERROR("Event type does not have the observer");
			return;
		}

		list.entries.erase(find_it);
	}
}