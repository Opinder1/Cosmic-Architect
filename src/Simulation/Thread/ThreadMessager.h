#pragma once

#include "ThreadOwnable.h"
#include "ThreadMessage.h"

#include "Simulation/Message/MessageSender.h"

#include "Simulation/Event/EventDispatcher.h"

#include <robin_hood/robin_hood.h>

#include <mutex>
#include <vector>
#include <atomic>

namespace sim
{
	class EventDispatcher;

	struct MessagerException {};

	// A messager that sends events to other messagers in this process. They could be in other threads
	class ThreadMessager : public EventDispatcher, public MessageSender, public ThreadOwnable
	{
	private:
		struct LinkedMessager
		{
			ThreadMessager* messager;
			ThreadMessageQueue queue;
		};

		using LinkedMessagerStorage = robin_hood::unordered_node_map<UUID, LinkedMessager>;

	public:
		explicit ThreadMessager(UUID id);
		~ThreadMessager();

		// Is this messager stopping
		bool IsStopping() const;

		// Send message to another messager. Run by user of this messager
		void PostMessageToOther(UUID target_id, const MessagePtr& message);

		// Send message to another messager. Run by user of this messager
		void PostMessagesToOther(UUID target_id, const MessageQueue& messages);

		// Send message to messager. Run by any thread
		void PostMessageFromUnattested(const MessagePtr& message);

		// Send messages to messager. Run by any thread
		void PostMessagesFromUnattested(const MessageQueue& messages);

	protected:
		// Link to another messager. Run by user of this messager
		void LinkMessager(ThreadMessager& target);

		// Remove the link to this messager. Run by user of this messager. The messager is not immediately unlinked so still expect some messages
		void UnlinkMessager(ThreadMessager& target);

		// Should be called when the thread that will own the messager starts messaging
		void DipatcherStart();

		// Should be called when the thread that owns the messager will stop messaging
		void DipatcherRequestStop();

		// Process all incomming messages. Always call before ProcessOutgoingMessages() to handle unlinks first
		void ProcessIncommingMessages();

		// Send all outgoing messages to the related messagers
		void ProcessOutgoingMessages();

	private:
		// Will be called when this dispatcher has cleaned up everything and can delete itself
		void DipatcherFinalStop();

		// Post a message but queue it to be handled next tick. Called internally by another messager
		void PostMessageFromOther(const ThreadMessage& message);

		// Post all messages in a queue to be handled in the next tick. Called internally by another messager
		void PostMessagesFromOther(const ThreadMessageQueue& messages);

		// On recieving a thread message. The message can be from this messager or send from another by a queue
		void OnThreadMessage(const ThreadMessage& message);

	private:
		// Mutex to protect m_in_queue
		mutable std::mutex m_mutex;

		// Used by all other messagers and should not interfere with this messagers message reading
		// Can be fought over by other messagers
		ThreadMessageQueue m_in_queue;

		// Is this messager stopping
		std::atomic_bool m_stopping;

		// We store a queue of messages to each linked messager to minimize thread locking
		LinkedMessagerStorage m_linked_messagers;
	};
}