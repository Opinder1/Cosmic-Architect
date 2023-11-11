#include "ThreadMessager.h"

#include "Simulation/Events.h"

#include "Util/Debug.h"

namespace sim
{
	ThreadMessager::ThreadMessager(UUID id) :
		MessageSender(id),
		m_stopping(false)
	{}

	ThreadMessager::~ThreadMessager()
	{}

	bool ThreadMessager::IsStopping() const
	{
		return m_stopping;
	}

	void ThreadMessager::LinkMessager(ThreadMessager& target)
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "The owning thread should be linking for this messager");

		if (&target == this)
		{
			DEBUG_PRINT_ERROR("Don't link to self");
			return;
		}

		if (m_stopping)
		{
			DEBUG_PRINT_ERROR("This messager is stopping");
			return;
		}
			
		// Post to the target. This must not fail
		target.PostMessageFromOther(ThreadMessage(ThreadMessage::Type::RequestLink, this, nullptr));
	}

	void ThreadMessager::UnlinkMessager(ThreadMessager& target)
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "The owning thread should be unlinking for this messager");

		if (&target == this)
		{
			DEBUG_PRINT_ERROR("Don't unlink to self");
			return;
		}

		// Post to the target. This must not fail
		target.PostMessageFromOther(ThreadMessage(ThreadMessage::Type::RequestUnlink, this, nullptr));
	}

	void ThreadMessager::PostMessageToOther(UUID target_id, const MessagePtr& message)
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "The owning thread should be posting messages for this messager");

		if (target_id == GetUUID())
		{
			DEBUG_PRINT_ERROR("Don't send to self");
			return;
		}

		auto it = m_linked_messagers.find(target_id);

		// Target should send to this
		if (it == m_linked_messagers.end())
		{
			DEBUG_PRINT_ERROR("Target dispatcher not linked");
			return;
		}

		ThreadMessageQueue& queue = it->second.queue;

		// Add to just this queue
		queue.push_back(ThreadMessage(ThreadMessage::Type::Message, this, message));
	}

	void ThreadMessager::PostMessagesToOther(UUID target_id, const MessageQueue& messages)
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "The owning thread should be posting messages for this messager");

		if (target_id == GetUUID())
		{
			DEBUG_PRINT_ERROR("Don't send to self");
			return;
		}

		auto it = m_linked_messagers.find(target_id);

		// Target should send to this
		if (it == m_linked_messagers.end())
		{
			DEBUG_PRINT_ERROR("Target dispatcher not linked");
			return;
		}

		ThreadMessageQueue& queue = it->second.queue;

		// Add to just this queue
		for (const std::shared_ptr<Message>& message : messages)
		{
			queue.push_back(ThreadMessage(ThreadMessage::Type::Message, this, message));
		}
	}

	void ThreadMessager::PostMessageFromUnattested(const MessagePtr& message)
	{
		if (ThreadOwnsObject()) // If we own this then don't bother queuing as we won't be reading the queue right now
		{
			PostEventGeneric(*message, GetMessageType(*message));
		}
		else
		{
			std::unique_lock lock(m_mutex);

			m_in_queue.push_back(ThreadMessage(ThreadMessage::Type::Message, nullptr, message));
		}
	}

	void ThreadMessager::PostMessagesFromUnattested(const MessageQueue& messages)
	{
		if (ThreadOwnsObject()) // If we own this then don't bother queuing as we won't be reading the queue right now
		{
			for (const std::shared_ptr<Message>& message : messages)
			{
				PostEventGeneric(*message, GetMessageType(*message));
			}
		}
		else
		{
			std::unique_lock lock(m_mutex);

			// Add to just this queue
			for (const std::shared_ptr<Message>& message : messages)
			{
				m_in_queue.push_back(ThreadMessage(ThreadMessage::Type::Message, nullptr, message));
			}
		}
	}

	void ThreadMessager::PostMessageFromOther(const ThreadMessage& message)
	{
		DEBUG_ASSERT(!ThreadOwnsObject(), "Should be called by another dispatcher. That dispatcher should be linked to call this");

		if (m_stopping)
		{
			DEBUG_PRINT_ERROR("This messager is stopping");
			return;
		}

		std::unique_lock lock(m_mutex);

		// Push only to the in queue so that this dispatcher can still run without blocking
		m_in_queue.push_back(message);
	}

	void ThreadMessager::PostMessagesFromOther(const ThreadMessageQueue& messages)
	{
		DEBUG_ASSERT(!ThreadOwnsObject(), "Should be called by another dispatcher. That dispatcher should be linked to call this");

		if (m_stopping)
		{
			DEBUG_PRINT_ERROR("This messager is stopping");
			return;
		}

		if (messages.empty()) // Don't lock mutex if there are no messages
		{
			return;
		}

		std::unique_lock lock(m_mutex);

		// Push only to the in queue so that this dispatcher can still run without blocking
		m_in_queue.insert(m_in_queue.end(), messages.begin(), messages.end());
	}

	void ThreadMessager::ProcessIncommingMessages()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "The owning thread should be processing messages for this messager");

		// Process the events sent to ourself first before messages sent from other messagers
		ProcessEventQueue();

		// Used exclusively by this messager and wont be blocked by other messagers sending to this
		ThreadMessageQueue process_queue;

		{
			std::unique_lock lock(m_mutex);

			// Swap the input queue and the process queue.
			// The dispatcher thread then has access to all the sent messages
			// The other dispatchers then have a fresh queue to input into
			m_in_queue.swap(process_queue);
		}

		if (!process_queue.empty())
		{
			// Process all messages in newly filled out queue that wont block other dispatchers adding to the in queue
			for (ThreadMessage& message : process_queue)
			{
				ProcessThreadMessage(message);
			}
		}

		// process_queue is deleted
	}

	void ThreadMessager::ProcessOutgoingMessages()
	{
		DEBUG_ASSERT(ThreadOwnsObject(), "The owning thread should be processing messages for this messager");

		// Forward all queues. This should also clear the queues when queues are posted
		for (auto&& [id, messager] : m_linked_messagers)
		{
			messager.messager->PostMessagesFromOther(messager.queue);

			messager.queue.clear();
		}

		// If we are stopping and we are not linked to anyone anymore then final stop
		// We want to do this at the end of a tick so its the last thing that happens before the loop stops
		if (m_stopping && m_linked_messagers.size() == 0)
		{
			// Tell the superclass we have stopped
			PostEvent(MessagerStopEvent());
		}
	}

	void ThreadMessager::MessagerStart()
	{
		DEBUG_ASSERT(m_stopping == false, "The dispatcher shouldn't be stopping");

		ThreadClaimObject(); // Claim this object so that ScheduleMessage requires a thread other than this one

		{
			std::unique_lock lock(m_mutex);

			// Clear anything that was sent to us when we were not active as its invalid
			m_in_queue.clear();
		}
	}

	void ThreadMessager::MessagerStop()
	{
		m_stopping = false;

		ThreadReleaseObject(); // Unclaim this object
	}

	void ThreadMessager::MessagerRequestStop()
	{
		// Make sure we own this. This is also checked in ThreadReleaseObject()
		DEBUG_ASSERT(ThreadOwnsObject(), "The owning thread should be stopping this messager");

		if (m_stopping) // If we are already stopping then ignore this call
		{
			DEBUG_PRINT_WARN("We are already stopping");
			return;
		}

		// Enable stopping mode where we make no new connections and attempt to unlink
		// from all connected messagers.
		m_stopping = true;

		// Send unlink requests to all linked dispatchers
		for (auto&& [id, messager] : m_linked_messagers)
		{
			messager.messager->PostMessageFromOther(ThreadMessage(ThreadMessage::Type::RequestUnlink, this, nullptr));
		}
	}

	void ThreadMessager::ProcessThreadMessage(const ThreadMessage& thread_message)
	{
		// Make sure we own this.
		DEBUG_ASSERT(ThreadOwnsObject(), "The owning thread should be handling messages for this messager");

		switch (thread_message.type)
		{
		case ThreadMessage::Type::RequestLink:
			// Don't make any new connections if we are stopping
			if (m_stopping)
			{
				DEBUG_PRINT_ERROR("This dispatcher is stopping so will allow no new connections");
				break;
			}

			if (m_linked_messagers.find(thread_message.sender->GetUUID()) == m_linked_messagers.end())
			{
				DEBUG_PRINT_ERROR("The target dispatcher is already linked");
				break;
			}

			m_linked_messagers.emplace(thread_message.sender->GetUUID(), LinkedMessager{ thread_message.sender });

			// Send the confirmation before the OnLink() callback. This is because
			// in that callback, messages to the newly linked messager could be sent
			// which should be processed after the queue is created in that messager.
			thread_message.sender->PostMessageFromOther(ThreadMessage(ThreadMessage::Type::ConfirmLink, this, nullptr));
			
			PostEvent(MessagerLinkEvent(thread_message.sender->GetUUID()));
			break;

		case ThreadMessage::Type::ConfirmLink:
			if (m_linked_messagers.find(thread_message.sender->GetUUID()) == m_linked_messagers.end())
			{
				DEBUG_PRINT_ERROR("The target dispatcher is already linked");
				break;
			}

			m_linked_messagers.emplace(thread_message.sender->GetUUID(), LinkedMessager{ thread_message.sender });

			PostEvent(MessagerLinkEvent(thread_message.sender->GetUUID()));
			break;

		case ThreadMessage::Type::RequestUnlink:
			if (m_linked_messagers.erase(thread_message.sender->GetUUID()) == 0)
			{
				DEBUG_PRINT_ERROR("The target dispatcher was not linked");
				break;
			}

			PostEvent(MessagerUnlinkEvent(thread_message.sender->GetUUID()));

			// Send the confirmation of the unlink after the OnUnlink() callback as
			// in that callback, some messages could be sent to that messager.
			thread_message.sender->PostMessageFromOther(ThreadMessage(ThreadMessage::Type::ConfirmUnlink, this, nullptr));
			break;

		case ThreadMessage::Type::ConfirmUnlink:
			if (m_linked_messagers.erase(thread_message.sender->GetUUID()) == 0)
			{
				DEBUG_PRINT_ERROR("The target dispatcher was not linked");
				break;
			}

			PostEvent(MessagerUnlinkEvent(thread_message.sender->GetUUID()));
			break;

		case ThreadMessage::Type::Message:
			PostEventGeneric(*thread_message.message, GetMessageType(*thread_message.message));
			break;

		default:
			DEBUG_PRINT_ERROR("Invalid ThreadMessage type");
			break;
		}
	}

	void ThreadMessager::OnAttemptFreeMemory(const AttemptFreeMemoryEvent& event)
	{
		std::unique_lock lock(m_mutex);

		m_in_queue.shrink_to_fit();
	}
}