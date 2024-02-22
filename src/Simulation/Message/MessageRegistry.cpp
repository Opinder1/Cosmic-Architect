#include "MessageRegistry.h"
#include "MessageSender.h"
#include "Message.h"

#include "Simulation/Stream/StreamHelper.h"

#include "Util/Debug.h"

#include <godot_cpp/variant/variant.hpp>

namespace sim
{
	MessageRegistry* MessageRegistry::s_singleton;

	//MessageRegistry* MessageRegistry::GetSingleton()
	//{
	//	DEBUG_ASSERT(s_singleton != nullptr, "THe singleton should be valid");
	//	return s_singleton;
	//}

	MessageRegistry::MessageRegistry()
	{
		if (s_singleton != nullptr)
		{
			DEBUG_PRINT_ERROR("Already initialised the message registry");
			return;
		}

		s_singleton = this;
	}

	MessageRegistry::~MessageRegistry()
	{
		s_singleton = nullptr;
	}

	void MessageRegistry::RegisterGenericMessageType(Event::Type type, std::unique_ptr<MessageFactoryBase>&& generator)
	{
		std::unique_lock lock(m_mutex);

		auto&& [it, success] = m_factories.emplace(type, std::move(generator));

		if (!success)
		{
			DEBUG_PRINT_ERROR("The message type '" + godot::String::num_uint64(type) + "' was already registered");
			return;
		}

		m_factory_list.push_back(it->second.get());
	}

	void MessageRegistry::RegisterMessageSender(const MessageSender& sender)
	{
		std::unique_lock lock(m_mutex);

		auto&& [it, success] = m_message_senders.emplace(sender.GetUUID(), &sender);

		if (!success)
		{
			DEBUG_PRINT_ERROR("A sender with the uuid already exists");
			return;
		}
	}

	void MessageRegistry::UnregisterMessageSender(const MessageSender& sender)
	{
		std::unique_lock lock(m_mutex);

		size_t num_erased = m_message_senders.erase(sender.GetUUID());

		if (num_erased != 1)
		{
			DEBUG_PRINT_ERROR("No sender with the requested uuid exists");
			return;
		}
	}

	bool MessageRegistry::PackMessage(const Message& message, ByteStream& stream, const MessageSender& target) const
	{
		Event::Type type = GetMessageType(message);

		uint32_t index;
		{
			std::shared_lock lock(m_mutex);

			auto it = m_factories.find(type);

			if (it == m_factories.end())
			{
				DEBUG_PRINT_ERROR("The message type '" + godot::String::num_uint64(type) + "' was not registered");
				return false;
			}

			index = it->second->GetIndex();
		}

		UUID sender_id = message.GetSender();

		UUID target_id = target.GetUUID();

		if (!StreamHelper::Write(stream, index))
		{
			DEBUG_PRINT_ERROR("Failed to write message index");
			return false;
		}

		if (!StreamHelper::WriteUUID(stream, sender_id))
		{
			DEBUG_PRINT_ERROR("Failed to write message sender uuid");
			return false;
		}

		if (!StreamHelper::WriteUUID(stream, target_id))
		{
			DEBUG_PRINT_ERROR("Failed to write message target uuid");
			return false;
		}

		if (!message.Serialize(stream))
		{
			DEBUG_PRINT_ERROR("Failed to serialise message");
			return false;
		}

		return true;
	}

	MessagePtr MessageRegistry::UnpackMessage(ByteStream& stream, UUID& target) const
	{
		uint32_t index;
		if (!StreamHelper::Read(stream, index))
		{
			DEBUG_PRINT_ERROR("Failed to read message index");
			return nullptr;
		}

		UUID sender_id;
		if (!StreamHelper::ReadUUID(stream, sender_id))
		{
			DEBUG_PRINT_ERROR("Failed to read message sender uuid");
			return nullptr;
		}

		UUID target_id;
		if (!StreamHelper::ReadUUID(stream, target_id))
		{
			DEBUG_PRINT_ERROR("Failed to read message target uuid");
			return nullptr;
		}

		MessagePtr message;
		{
			std::shared_lock lock(m_mutex);

			if (index >= m_factory_list.size())
			{
				DEBUG_PRINT_ERROR("The message index '" + godot::String::num_uint64(index) + "' is out of range");
				return nullptr;
			}

			auto sender_it = m_message_senders.find(sender_id);

			if (sender_it == m_message_senders.end())
			{
				DEBUG_PRINT_ERROR("The messages sender uuid is invalid");
				return nullptr;
			}

			message = m_factory_list[index]->CreateMessage(*sender_it->second);
		}

		if (message == nullptr)
		{
			DEBUG_PRINT_ERROR("Factory returned invalid message");
			return nullptr;
		}

		if (!message->Deserialize(stream))
		{
			DEBUG_PRINT_ERROR("Failed to deserialise message");
			return nullptr;
		}

		target = target_id;

		return message;
	}
}