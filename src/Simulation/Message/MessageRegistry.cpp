#include "MessageRegistry.h"

#include "Simulation/Stream/StreamHelper.h"

#include "Simulation/Message/MessageSender.h"
#include "Simulation/Message/Message.h"

#include <godot_cpp/variant/variant.hpp>

#include <godot_cpp/core/error_macros.hpp>

namespace sim
{
	std::unique_ptr<MessageRegistry> MessageRegistry::s_singleton;

	void MessageRegistry::Initialize()
	{
		CRASH_COND(s_singleton != nullptr);

		s_singleton = std::make_unique<MessageRegistry>();
	}

	void MessageRegistry::Uninitialize()
	{
		s_singleton.reset();
	}

	MessageRegistry* MessageRegistry::GetSingleton()
	{
		return s_singleton.get();
	}

	MessageRegistry::MessageRegistry()
	{}

	MessageRegistry::~MessageRegistry()
	{}

	void MessageRegistry::RegisterGenericType(Event::Type type, std::unique_ptr<MessageFactoryBase>&& generator)
	{
		std::unique_lock lock(m_mutex);

		auto&& [it, success] = m_factories.emplace(type, std::move(generator));

		ERR_FAIL_COND_MSG(!success, "The message type '" + godot::String::num_uint64(type) + "' was already registered");

		m_factory_list.push_back(it->second.get());
	}

	void MessageRegistry::RegisterMessageSender(const MessageSender& sender)
	{
		std::unique_lock lock(m_mutex);

		auto&& [it, success] = m_message_senders.emplace(sender.GetUUID(), &sender);

		ERR_FAIL_COND_MSG(!success, "A sender with the uuid already exists");
	}

	void MessageRegistry::UnregisterMessageSender(const MessageSender& sender)
	{
		std::unique_lock lock(m_mutex);

		size_t num_erased = m_message_senders.erase(sender.GetUUID());

		ERR_FAIL_COND_MSG(num_erased != 1, "No sender with the requested uuid exists");
	}

	bool MessageRegistry::PackMessage(const Message& message, ByteStream& stream, const MessageSender& target) const
	{
		Event::Type type = GetMessageType(message);

		uint32_t index;
		{
			std::shared_lock lock(m_mutex);

			auto it = m_factories.find(type);
			ERR_FAIL_COND_V_MSG(it == m_factories.end(), false, "The message type '" + godot::String::num_uint64(type) + "' was not registered");

			index = it->second->GetIndex();
		}

		UUID sender_id = message.GetSender();

		UUID target_id = target.GetUUID();

		ERR_FAIL_COND_V_MSG(!StreamHelper::Write(stream, index), false, "Failed to write message index");

		ERR_FAIL_COND_V_MSG(!StreamHelper::WriteUUID(stream, sender_id), false, "Failed to write message sender uuid");

		ERR_FAIL_COND_V_MSG(!StreamHelper::WriteUUID(stream, target_id), false, "Failed to write message target uuid");

		return message.Serialize(stream);
	}

	MessagePtr MessageRegistry::UnpackMessage(ByteStream& stream, UUID& target) const
	{
		uint32_t index;
		ERR_FAIL_COND_V_MSG(!StreamHelper::Read(stream, index), nullptr, "Failed to read message index");

		UUID sender_id;
		ERR_FAIL_COND_V_MSG(!StreamHelper::ReadUUID(stream, sender_id), nullptr, "Failed to read message sender uuid");

		UUID target_id;
		ERR_FAIL_COND_V_MSG(!StreamHelper::ReadUUID(stream, target_id), nullptr, "Failed to read message index");

		MessagePtr message;
		{
			std::shared_lock lock(m_mutex);

			ERR_FAIL_COND_V_MSG(index >= m_factory_list.size(), nullptr, "The message index '" + godot::String::num_uint64(index) + "' is out of range");

			auto sender_it = m_message_senders.find(sender_id);
			ERR_FAIL_COND_V_MSG(sender_it == m_message_senders.end(), nullptr, "The messages sender uuid is invalid");

			target = target_id;

			message = m_factory_list[index]->CreateMessage(*sender_it->second);
		}

		if (!message->Deserialize(stream))
		{
			return nullptr;
		}

		return message;
	}
}