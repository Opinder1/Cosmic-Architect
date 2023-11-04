#include "System.h"
#include "Simulation.h"

#include "UUID.h"

namespace sim
{
	System::System(Simulation& simulation) :
		m_simulation(simulation),
		m_registry(simulation.Registry())
	{

	}

	System::~System()
	{
		for (CallbackEntry& entry : m_callbacks)
		{
			Sim().UnsubscribeGeneric(entry.callback, entry.type);
		}
	}

	Simulation& System::Sim()
	{
		return m_simulation;
	}

	entt::registry& System::Registry()
	{
		return m_registry;
	}

	UUID System::GenerateUUID()
	{
		return m_simulation.GenerateUUID();
	}
}