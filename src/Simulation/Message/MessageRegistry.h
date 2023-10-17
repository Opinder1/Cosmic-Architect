#pragma once

#include "Message.h"
#include "MessageFactory.h"

#include "Simulation/UUID.h"

#include <robin_hood/robin_hood.h>

#include <TKRZW/tkrzw_thread_util.h>

namespace sim
{
	class ByteStream;
	class MessageFactoryBase;
	class MessageSender;

	// A producer of events of which their ids are incremental
	class MessageRegistry
	{
	public:
		static void Initialize();
		static void Uninitialize();
		static MessageRegistry* GetSingleton();

		explicit MessageRegistry();
		~MessageRegistry();

		// Register a new event type that can be generated. Make sure to call this in order for each app that will communicate so ids are correct.
		template<class MessageT>
		void RegisterType()
		{
			static_assert(std::is_base_of_v<Message, MessageT>);

			// The index of each message are set incrementally in the order they are registered
			uint32_t index = uint32_t(m_factory_list.size()); // If we reach UINT32_MAX then we are doing something wrong

			RegisterGenericType(GetEventType<MessageT>(), std::make_unique<MessageFactory<MessageT>>(index));
		}

		// Called by a message sender when it initializes itself
		void RegisterMessageSender(const MessageSender& sender);

		// Called by a message sender when it deinitializes itself
		void UnregisterMessageSender(const MessageSender& sender);

		// Serialize a message to a stream
		bool PackMessage(const Message& message, ByteStream& stream, const MessageSender& target) const;

		// Deserialize a message from a stream
		MessagePtr UnpackMessage(ByteStream& stream, UUID& target) const;

	private:
		void RegisterGenericType(Message::Type type, std::unique_ptr<MessageFactoryBase>&& factory);

	private:
		static std::unique_ptr<MessageRegistry> s_singleton;

		mutable tkrzw::SpinSharedMutex m_mutex;

		robin_hood::unordered_flat_map<Message::Type, std::unique_ptr<MessageFactoryBase>> m_factories;

		robin_hood::unordered_flat_map<UUID, const MessageSender*> m_message_senders;

		std::vector<MessageFactoryBase*> m_factory_list;
	};
}