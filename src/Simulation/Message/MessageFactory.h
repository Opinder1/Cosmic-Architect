#pragma once

#include "Message.h"

namespace sim
{
	class MessageSender;

	// Message factory base class that creates a message for one type
	class MessageFactoryBase
	{
	public:
		MessageFactoryBase(Message::Type type, uint32_t index);

		virtual ~MessageFactoryBase();

		// Get the type of the message which is usually the class hash
		Message::Type GetType() const;

		// Get the index of the message. This index should be agreed upon by all recipients that are communicating with this factory
		uint32_t GetIndex() const;

		// Create a new message of the type the derived factory is for
		virtual MessagePtr CreateMessage(const MessageSender& sender) = 0;

	private:
		Message::Type m_type;
		uint32_t m_index;
	};

	// A generic factory that can be templated for all types
	template<class MessageT>
	class MessageFactory : public MessageFactoryBase
	{
	public:
		explicit MessageFactory(uint32_t index) : MessageFactory(GetEventType<MessageT>(), index) {}

		MessagePtr CreateMessage(const MessageSender& sender) override
		{
			return std::make_shared<MessageT>(sender);
		}
	};
}