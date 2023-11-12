#pragma once

#include "Simulation/Message/Message.h"

#include <memory>

namespace sim
{
	class ThreadMessager;

	// An event that should be sent between simulations over threads
	struct ThreadMessage
	{
		enum class Type
		{
			RequestLink,
			ConfirmLink,
			RequestUnlink,
			ConfirmUnlink,
			Unlink,
			Message,
			Invalid
		};

		explicit ThreadMessage() :
			type(Type::Invalid),
			sender(nullptr),
			message(nullptr)
		{}

		explicit ThreadMessage(Type type, ThreadMessager* sender, const MessagePtr& message) :
			type(type),
			sender(sender),
			message(message)
		{}

		Type type;
		ThreadMessager* sender;
		MessagePtr message;
	};

	struct ThreadMessageQueue : std::vector<ThreadMessage>
	{
		using std::vector<ThreadMessage>::vector;
	};
}