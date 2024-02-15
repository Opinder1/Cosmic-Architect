#include "SimulationTestNode.h"

#include "Simulation/SimulationMessager.h"
#include "Simulation/SimulationServer.h"
#include "Simulation/Events.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/resource_uid.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace
{
	void SimulationTickCallback(sim::SimulationMessager* simulation, const sim::SimulationTickEvent& event)
	{
		if (simulation->GetTotalTicks() % 1000 == 0)
		{
			auto id = std::this_thread::get_id();
			godot::UtilityFunctions::print(godot::vformat("Thread %d on tick %d", *(unsigned int*)&id, simulation->GetTotalTicks()));
		}
	}
}

SimulationTestNode::SimulationTestNode() :
	m_try_and_add(false),
	m_added(false)
{
}

SimulationTestNode::~SimulationTestNode()
{
}

void SimulationTestNode::_notification(int notification)
{
	switch (notification)
	{
	case NOTIFICATION_ENTER_TREE:
		m_id = Create("");

		if (!Start(m_id))
		{
			DEBUG_PRINT_ERROR("We failed to start the simulation we just created");
		}

		m_try_and_add = true;
		break;

	case NOTIFICATION_EXIT_TREE:
		if (sim::SimulationMessager* messager = GetAcquiredSimulation(m_id))
		{
			messager->Unsubscribe(cb::BindParam<&SimulationTickCallback>(messager));

		}

		m_try_and_add = false;

		if (m_added)
		{
			if (!Remove(m_id))
			{
				DEBUG_PRINT_ERROR("We failed to remove the simulation that we created and should have added to ourselves");
			}
			m_added = false;
		}
		
		if (!m_id.is_empty())
		{
			Delete(m_id);
		}
		break;

	case NOTIFICATION_PROCESS:
		if (m_try_and_add)
		{
			if (Add(m_id))
			{
				m_try_and_add = false;
				m_added = true;
			}
		}

		if (sim::SimulationMessager* messager = GetAcquiredSimulation(m_id))
		{
			messager->Subscribe(cb::BindParam<&SimulationTickCallback>(messager));
		}
		break;
	}
}

void SimulationTestNode::_bind_methods()
{

}