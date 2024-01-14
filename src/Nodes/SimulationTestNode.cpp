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

SimulationTestNode::SimulationTestNode() 
{
	Start();

	ThreadAcquire();

	if (sim::SimulationMessager* messager = GetAcquiredSimulation())
	{
		messager->Subscribe(cb::BindParam<&SimulationTickCallback>(messager));
	}
}

SimulationTestNode::~SimulationTestNode()
{
	if (sim::SimulationMessager* messager = GetAcquiredSimulation())
	{
		messager->Unsubscribe(cb::BindParam<&SimulationTickCallback>(messager));

		ThreadRelease();
	}

}

void SimulationTestNode::_bind_methods()
{

}