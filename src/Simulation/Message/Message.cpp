#include "Message.h"

#include "MessageSender.h"

#include <godot_cpp/variant/variant.hpp>

#include <godot_cpp/core/error_macros.hpp>

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
		ERR_FAIL_V_MSG(false, godot::vformat("Serialize() was called on a message of type '%s' that does not override it.", typeid(*this).name()));
	};

	bool Message::Deserialize(ByteStream& stream)
	{
		ERR_FAIL_V_MSG(false, godot::vformat("Deserialize() was called on a message of type '%s' that does not override it.", typeid(*this).name()));
	};

	Event::Type GetMessageType(const Message& message)
	{
		return typeid(message).hash_code();
	}
}