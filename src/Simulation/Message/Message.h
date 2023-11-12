#pragma once

#include "Simulation/Event/Event.h"

#include "Simulation/UUID.h"

#include <memory>
#include <vector>

namespace sim
{
	class ByteStream;
	class MessageSender;

	// A message that can be sent between unrelated objects with no direct pointer/reference of each other.
	// Its members may be read by multiple threads at once so should be thread safe
	struct Message : Event
	{
		explicit Message(const MessageSender& sender);

		virtual ~Message() = 0;

		UUID GetSender() const;

		virtual bool Serialize(ByteStream& stream) const;
		virtual bool Deserialize(ByteStream& stream) const;

	private:
		UUID sender; // UUID of the MessageSender that sent this message
	};

	// A pointer to a message. Is a shared pointer since we may send it to multiple targets on different threads
	struct MessagePtr : std::shared_ptr<const Message>
	{
		using shared_ptr<const Message>::shared_ptr;
	};

	struct MessageQueue : std::vector<MessagePtr>
	{
		using vector<MessagePtr>::vector;
	};

	// Get the message type from a generic message
	Event::Type GetMessageType(const Message& message);
}