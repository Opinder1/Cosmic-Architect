#include "Message.h"

#include "MessageSender.h"

#include "Util/Debug.h"

#include <godot_cpp/variant/variant.hpp>

namespace sim
{
	Message::Message(const MessageSender& sender) :
		sender(sender.GetUUID())
	{}

	Message::~Message()
	{}

	UUID Message::GetSender() const
	{
		return sender;
	}

	bool Message::Serialize(ByteStream& stream) const
	{
		DEBUG_PRINT_ERROR(godot::vformat("Serialize() was called on a message of type '%s' that does not override it.", typeid(*this).name()));
		return false;
	};

	bool Message::Deserialize(ByteStream& stream)
	{
		DEBUG_PRINT_ERROR(godot::vformat("Deserialize() was called on a message of type '%s' that does not override it.", typeid(*this).name()));
		return false;
	};

	Event::Type GetMessageType(const Message& message)
	{
		return typeid(message).hash_code();
	}
}