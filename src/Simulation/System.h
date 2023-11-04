#pragma once

#include "Util/Nocopy.h"

#include "Simulation.h"

namespace sim
{
	class Simulation;
	class UUID;

	struct SimulationRequestStopMessage;
	struct SimulationStopEvent;

	class System : nocopy
	{
		struct CallbackEntry
		{
			EventCallback<Event> callback;
			Event::Type type;
		};

	public:
		System(Simulation& simulation);
		virtual ~System() = 0;

		// Simulation accessor with easy to type name
		Simulation& Sim();

		// Registry accessor with easy to type name
		entt::registry& Registry();

		template<class T>
		T& Global() { return Sim().Global<T>(); }

		template<auto MethodT>
		void Subscribe(Priority priority = Priority::Normal)
		{
			SubscribeHelper<MethodT>(MethodT, priority);
		}

		UUID GenerateUUID();

	private:
		template<auto MethodT, class Class, class Ret, class EventT>
		void SubscribeHelper(Ret(Class::*)(const EventT&), Priority priority)
		{
			EventCallback<EventT> callback = cb::Bind<MethodT>(static_cast<Class*>(this));

			Event::Type type = GetEventType<EventT>();

			Sim().SubscribeGeneric(GetGenericCallback(callback), type, priority);

			m_callbacks.emplace_back(CallbackEntry{ GetGenericCallback(callback), type });
		}

	private:
		Simulation& m_simulation;

		entt::registry& m_registry;

		std::vector<CallbackEntry> m_callbacks;
	};
}