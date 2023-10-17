#include "EventDispatcher.h"

#include <godot_cpp/variant/string.hpp>

#include <godot_cpp/core/error_macros.hpp>

namespace sim
{
	EventDispatcher::EventDispatcher()
	{

	}

	EventDispatcher::~EventDispatcher()
	{
#if _DEBUG
		for (auto&& [type, callback_list] : m_callback_lists)
		{
			if (!callback_list.empty())
			{
				WARN_PRINT("The callback list for the type '" + godot::String::num_uint64(type) + "' was not empty on destruction");
			}
		}
#endif
	}

	void EventDispatcher::ProcessEventQueue()
	{
		for (QueueEntry& entry : m_queue)
		{
			PostEvent(*entry.event, entry.type);
		}

		m_queue.clear();
	}

	void EventDispatcher::PostEvent(const Event& event, Event::Type event_type)
	{
		auto it = m_callback_lists.find(event_type);

		if (it != m_callback_lists.end())
		{
			PostEventInternal(it->second, event);
		}
	}

	void EventDispatcher::PostQueuedEvent(EventPtr&& event, Event::Type event_type)
	{
		m_queue.emplace_back(QueueEntry{ std::move(event), event_type });
	}

	void EventDispatcher::SubscribeGeneric(const EventCallback<Event>& callback, Event::Type event_type, Priority priority)
	{
		SubscribeInternal(m_callback_lists[event_type], callback, priority);
	}

	void EventDispatcher::UnsubscribeGeneric(const EventCallback<Event>& callback, Event::Type event_type)
	{
		auto it = m_callback_lists.find(event_type);

		ERR_FAIL_COND_MSG(it == m_callback_lists.end(), "Event type '" + godot::String::num_uint64(event_type) + "' has no observers so unsubscribe failed");

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

	void EventDispatcher::SubscribeInternal(CallbackList& list, const EventCallback<Event>& callback, Priority priority)
	{
		// Here we emplace after the first item we find that is higher priority than this callback
		auto it = std::find_if(list.begin(), list.end(), [&](CallbackEntry& other) { return other.priority < priority; });

		// We should always emplace even if we hit the end
		list.emplace(it, CallbackEntry{ callback, priority });
	}

	void EventDispatcher::UnsubscribeInternal(CallbackList& list, const EventCallback<Event>& callback)
	{
		auto find_it = std::find_if(list.begin(), list.end(), [&](CallbackEntry& other) { return other.callback == callback; });

		ERR_FAIL_COND_MSG(find_it == list.end(), "Event type does not have the observer");

		list.erase(find_it);
	}
}