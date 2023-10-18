#include "ThreadMessager.h"

#include "Simulation/Events.h"

#include "Simulation/Event/EventDispatcher.h"

#include <godot_cpp/core/error_macros.hpp>

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
		ERR_FAIL_COND_MSG(!ThreadOwnsObject(), "The owning thread should be linking for this messager");
		ERR_FAIL_COND_MSG(&target == this, "Don't link to self");
		ERR_FAIL_COND_MSG(m_stopping, "This messager is stopping");
			
		// Post to the target. This must not fail
		target.PostMessageFromOther(ThreadMessage(ThreadMessage::Type::RequestLink, this, nullptr));
	}

	void ThreadMessager::UnlinkMessager(ThreadMessager& target)
	{
		ERR_FAIL_COND_MSG(!ThreadOwnsObject(), "The owning thread should be unlinking for this messager");
		ERR_FAIL_COND_MSG(&target == this, "Don't unlink from self");

		// Post to the target. This must not fail
		target.PostMessageFromOther(ThreadMessage(ThreadMessage::Type::RequestUnlink, this, nullptr));
	}

	void ThreadMessager::PostMessageToOther(UUID target_id, const MessagePtr& message)
	{
		ERR_FAIL_COND_MSG(!ThreadOwnsObject(), "The owning thread should be posting messages for this messager");
		ERR_FAIL_COND_MSG(target_id == GetUUID(), "Don't send to self");

		auto it = m_linked_messagers.find(target_id);

		// Target should send to this
		ERR_FAIL_COND_MSG(it == m_linked_messagers.end(), "Target dispatcher not linked");

		ThreadMessageQueue& queue = it->second.queue;

		// Add to just this queue
		queue.push_back(ThreadMessage(ThreadMessage::Type::Message, this, message));
	}

	void ThreadMessager::PostMessagesToOther(UUID target_id, const MessageQueue& messages)
	{
		ERR_FAIL_COND_MSG(!ThreadOwnsObject(), "The owning thread should be posting messages for this messager");
		ERR_FAIL_COND_MSG(target_id == GetUUID(), "Don't send to self");

		auto it = m_linked_messagers.find(target_id);

		// Target should send to this
		ERR_FAIL_COND_MSG(it == m_linked_messagers.end(), "Target dispatcher not linked");

		ThreadMessageQueue& queue = it->second.queue;

		// Add to just this queue
		for (const std::shared_ptr<Message>& message : messages)
		{
			queue.push_back(ThreadMessage(ThreadMessage::Type::Message, this, message));
		}
	}

	void ThreadMessager::PostMessageFromUnattested(const MessagePtr& message)
	{
		std::unique_lock lock(m_mutex);

		m_in_queue.push_back(ThreadMessage(ThreadMessage::Type::DirectMessage, nullptr, message));
	}

	void ThreadMessager::PostMessagesFromUnattested(const MessageQueue& messages)
	{
		std::unique_lock lock(m_mutex);

		// Add to just this queue
		for (const std::shared_ptr<Message>& message : messages)
		{
			m_in_queue.push_back(ThreadMessage(ThreadMessage::Type::DirectMessage, nullptr, message));
		}
	}

	void ThreadMessager::PostMessageFromOther(const ThreadMessage& message)
	{
		ERR_FAIL_COND_MSG(ThreadOwnsObject(), "Should be called by another dispatcher. That dispatcher should be linked to call this");
		ERR_FAIL_COND_MSG(m_stopping, "This messager is stopping");

		std::unique_lock lock(m_mutex);

		// Push only to the in queue so that this dispatcher can still run without blocking
		m_in_queue.push_back(message);
	}

	void ThreadMessager::PostMessagesFromOther(const ThreadMessageQueue& messages)
	{
		ERR_FAIL_COND_MSG(ThreadOwnsObject(), "Should be called by another dispatcher. That dispatcher should be linked to call this");
		ERR_FAIL_COND_MSG(m_stopping, "This messager is stopping");

		if (!messages.empty()) // Don't lock mutex if there are no messages
		{
			std::unique_lock lock(m_mutex);

			// Push only to the in queue so that this dispatcher can still run without blocking
			for (const ThreadMessage& message : messages)
			{
				m_in_queue.push_back(message);
			}
		}
	}

	void ThreadMessager::ProcessIncommingMessages()
	{
		ERR_FAIL_COND_MSG(!ThreadOwnsObject(), "The owning thread should be processing messages for this messager");

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
				OnThreadMessage(message);
			}
		}

		// process_queue is deleted
	}

	void ThreadMessager::ProcessOutgoingMessages()
	{
		ERR_FAIL_COND_MSG(!ThreadOwnsObject(), "The owning thread should be processing messages for this messager");

		// Process the events sent to ourself because we may create outgoing messages in them
		ProcessEventQueue();

		// Forward all queues. This should also clear the queues when queues are posted
		for (auto&& [id, messager] : m_linked_messagers)
		{
			messager.messager->PostMessagesFromOther(messager.queue);

			messager.queue.clear();
		}

		if (m_stopping && m_linked_messagers.size() == 0) // If we are stopping and we are not linked to anyone anymore
		{
			DipatcherFinalStop();
		}
	}

	void ThreadMessager::DipatcherStart()
	{
		ThreadClaimObject(); // Claim this object so that ScheduleMessage requires a thread other than this one

		// Clear anything that was sent to us when we were not active as its invalid
		m_in_queue.clear();
	}

	void ThreadMessager::DipatcherRequestStop()
	{
		// Make sure we own this. This is also checked in ThreadReleaseObject()
		ERR_FAIL_COND_MSG(!ThreadOwnsObject(), "The owning thread should be stopping this messager");

		if (m_stopping) // If we are already stopping then ignore this call
		{
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

	void ThreadMessager::DipatcherFinalStop()
	{
		// Tell the superclass we have stopped
		PostEvent(MessagerStopEvent());

		m_stopping = false;

		ThreadReleaseObject(); // Unclaim this object
	}

	void ThreadMessager::OnThreadMessage(const ThreadMessage& thread_message)
	{
		// Make sure we own this.
		ERR_FAIL_COND_MSG(!ThreadOwnsObject(), "The owning thread should be handling messages for this messager");

		switch (thread_message.type)
		{
		case ThreadMessage::Type::RequestLink:
			// Don't make any new connections if we are stopping
			ERR_FAIL_COND_MSG(m_stopping, "This dispatcher is stopping so will allow no new connections");

			ERR_FAIL_COND_MSG(m_linked_messagers.find(thread_message.sender->GetUUID()) == m_linked_messagers.end(), "The target dispatcher is already linked");

			m_linked_messagers.emplace(thread_message.sender->GetUUID(), LinkedMessager{ thread_message.sender });

			// Send the confirmation before the OnLink() callback. This is because
			// in that callback, messages to the newly linked messager could be sent
			// which should be processed after the queue is created in that messager.
			thread_message.sender->PostMessageFromOther(ThreadMessage(ThreadMessage::Type::ConfirmLink, this, nullptr));
			
			PostEvent(MessagerLinkEvent(thread_message.sender->GetUUID()));
			break;

		case ThreadMessage::Type::ConfirmLink:
			ERR_FAIL_COND_MSG(m_linked_messagers.find(thread_message.sender->GetUUID()) == m_linked_messagers.end(), "The target dispatcher is already linked");

			m_linked_messagers.emplace(thread_message.sender->GetUUID(), LinkedMessager{ thread_message.sender });

			PostEvent(MessagerLinkEvent(thread_message.sender->GetUUID()));
			break;

		case ThreadMessage::Type::RequestUnlink:
			if (m_linked_messagers.erase(thread_message.sender->GetUUID()) == 0)
			{
				ERR_FAIL_MSG("The target dispatcher was not linked");
			}

			PostEvent(MessagerUnlinkEvent(thread_message.sender->GetUUID()));

			// Send the confirmation of the unlink after the OnUnlink() callback as
			// in that callback, some messages could be sent to that messager.
			thread_message.sender->PostMessageFromOther(ThreadMessage(ThreadMessage::Type::ConfirmUnlink, this, nullptr));
			break;

		case ThreadMessage::Type::ConfirmUnlink:
			if (m_linked_messagers.erase(thread_message.sender->GetUUID()) == 0)
			{
				ERR_FAIL_MSG("The target dispatcher was not linked");
			}

			PostEvent(MessagerUnlinkEvent(thread_message.sender->GetUUID()));
			break;

		case ThreadMessage::Type::Message:
			PostEvent(*thread_message.message, GetMessageType(*thread_message.message));
			break;

		case ThreadMessage::Type::DirectMessage:
			PostEvent(*thread_message.message, GetMessageType(*thread_message.message));
			break;

		default:
			ERR_FAIL_MSG("Invalid ThreadMessage type");
			break;
		}
	}
}