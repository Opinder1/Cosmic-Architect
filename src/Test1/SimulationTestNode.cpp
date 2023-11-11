#include "SimulationTestNode.h"

#include "Simulation/Simulation.h"
#include "Simulation/SimulationServer.h"
#include "Simulation/Events.h"

#include <godot_cpp/variant/utility_functions.hpp>

namespace
{
	void SimulationTickCallback(sim::Simulation* simulation, const sim::TickEvent& event)
	{
		if (simulation->GetTotalTicks() % size_t(simulation->GetTicksPerSecond() * 10) == 0)
		{
			auto id = std::this_thread::get_id();
			godot::UtilityFunctions::print(godot::vformat("Thread %d on tick %d", *(unsigned int*)&id, simulation->GetTotalTicks()));
		}
	}
}

SimulationTestNode::SimulationTestNode()
{
	set_notify_transform(true);
	set_notify_local_transform(true);

	m_simulation = sim::SimulationServer::GetSingleton()->CreateSimulation(60, true);
	
	sim::SimulationServer::GetSingleton()->ApplyToSimulation(m_simulation, [](sim::Simulation& simulation)
	{
		simulation.Subscribe(cb::BindParam<&SimulationTickCallback>(&simulation));
	});

	sim::SimulationServer::GetSingleton()->StartSimulation(m_simulation, true);
}

SimulationTestNode::~SimulationTestNode()
{
	sim::SimulationServer::GetSingleton()->ApplyToSimulation(m_simulation, [](sim::Simulation& simulation)
	{
		simulation.Unsubscribe(cb::BindParam<&SimulationTickCallback>(&simulation));
	});

	sim::SimulationServer::GetSingleton()->StopAndDeleteSimulation(m_simulation);
}

void SimulationTestNode::_input(const godot::Ref<godot::InputEvent>& event)
{
	godot::UtilityFunctions::print("_input ", event->as_text());
}

void SimulationTestNode::_shortcut_input(const godot::Ref<godot::InputEvent>& event)
{
	godot::UtilityFunctions::print("_shortcut_input ", event->as_text());
}

void SimulationTestNode::_unhandled_input(const godot::Ref<godot::InputEvent>& event)
{
	godot::UtilityFunctions::print("_unhandled_input ", event->as_text());
}

void SimulationTestNode::_unhandled_key_input(const godot::Ref<godot::InputEvent>& event)
{
	godot::UtilityFunctions::print("_unhandled_key_input ", event->as_text());
}

void SimulationTestNode::_notification(int notification)
{
	switch (notification)
	{
	case NOTIFICATION_POSTINITIALIZE:
		godot::UtilityFunctions::print("NOTIFICATION_POSTINITIALIZE");
		break;

	case NOTIFICATION_PREDELETE:
		godot::UtilityFunctions::print("NOTIFICATION_PREDELETE");
		break;

	case NOTIFICATION_ENTER_TREE:
		godot::UtilityFunctions::print("NOTIFICATION_ENTER_TREE");
		break;

	case NOTIFICATION_EXIT_TREE:
		godot::UtilityFunctions::print("NOTIFICATION_EXIT_TREE");
		break;

	case NOTIFICATION_MOVED_IN_PARENT:
		godot::UtilityFunctions::print("NOTIFICATION_MOVED_IN_PARENT");
		break;

	case NOTIFICATION_READY:
		godot::UtilityFunctions::print("NOTIFICATION_READY");
		break;

	case NOTIFICATION_PAUSED:
		godot::UtilityFunctions::print("NOTIFICATION_PAUSED");
		break;

	case NOTIFICATION_UNPAUSED:
		godot::UtilityFunctions::print("NOTIFICATION_UNPAUSED");
		break;

	case NOTIFICATION_PHYSICS_PROCESS:
		godot::UtilityFunctions::print("NOTIFICATION_PHYSICS_PROCESS");
		break;

	case NOTIFICATION_PROCESS:
		godot::UtilityFunctions::print("NOTIFICATION_PROCESS");
		sim::SimulationServer::GetSingleton()->TickSimulation(m_simulation);
		break;

	case NOTIFICATION_PARENTED:
		godot::UtilityFunctions::print("NOTIFICATION_PARENTED");
		break;

	case NOTIFICATION_UNPARENTED:
		godot::UtilityFunctions::print("NOTIFICATION_UNPARENTED");
		break;

	case NOTIFICATION_SCENE_INSTANTIATED:
		godot::UtilityFunctions::print("NOTIFICATION_SCENE_INSTANTIATED");
		break;

	case NOTIFICATION_DRAG_BEGIN:
		godot::UtilityFunctions::print("NOTIFICATION_DRAG_BEGIN");
		break;

	case NOTIFICATION_DRAG_END:
		godot::UtilityFunctions::print("NOTIFICATION_DRAG_END");
		break;

	case NOTIFICATION_PATH_RENAMED:
		godot::UtilityFunctions::print("NOTIFICATION_PATH_RENAMED");
		break;

	case NOTIFICATION_CHILD_ORDER_CHANGED:
		godot::UtilityFunctions::print("NOTIFICATION_CHILD_ORDER_CHANGED");
		break;

	case NOTIFICATION_INTERNAL_PROCESS:
		godot::UtilityFunctions::print("NOTIFICATION_INTERNAL_PROCESS");
		break;

	case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
		godot::UtilityFunctions::print("NOTIFICATION_INTERNAL_PHYSICS_PROCESS");
		break;

	case NOTIFICATION_POST_ENTER_TREE:
		godot::UtilityFunctions::print("NOTIFICATION_POST_ENTER_TREE");
		break;

	case NOTIFICATION_DISABLED:
		godot::UtilityFunctions::print("NOTIFICATION_DISABLED");
		break;

	case NOTIFICATION_ENABLED:
		godot::UtilityFunctions::print("NOTIFICATION_ENABLED");
		break;

	case NOTIFICATION_NODE_RECACHE_REQUESTED:
		godot::UtilityFunctions::print("NOTIFICATION_NODE_RECACHE_REQUESTED");
		break;

	case NOTIFICATION_EDITOR_PRE_SAVE:
		godot::UtilityFunctions::print("NOTIFICATION_EDITOR_PRE_SAVE");
		break;

	case NOTIFICATION_EDITOR_POST_SAVE:
		godot::UtilityFunctions::print("NOTIFICATION_EDITOR_POST_SAVE");
		break;

	case NOTIFICATION_WM_MOUSE_ENTER:
		godot::UtilityFunctions::print("NOTIFICATION_WM_MOUSE_ENTER");
		break;

	case NOTIFICATION_WM_MOUSE_EXIT:
		godot::UtilityFunctions::print("NOTIFICATION_WM_MOUSE_EXIT");
		break;

	case NOTIFICATION_WM_WINDOW_FOCUS_IN:
		godot::UtilityFunctions::print("NOTIFICATION_WM_WINDOW_FOCUS_IN");
		break;

	case NOTIFICATION_WM_WINDOW_FOCUS_OUT:
		godot::UtilityFunctions::print("NOTIFICATION_WM_WINDOW_FOCUS_OUT");
		break;

	case NOTIFICATION_WM_CLOSE_REQUEST:
		godot::UtilityFunctions::print("NOTIFICATION_WM_CLOSE_REQUEST");
		break;

	case NOTIFICATION_WM_GO_BACK_REQUEST:
		godot::UtilityFunctions::print("NOTIFICATION_WM_GO_BACK_REQUEST");
		break;

	case NOTIFICATION_WM_SIZE_CHANGED:
		godot::UtilityFunctions::print("NOTIFICATION_WM_SIZE_CHANGED");
		break;

	case NOTIFICATION_WM_DPI_CHANGE:
		godot::UtilityFunctions::print("NOTIFICATION_WM_DPI_CHANGE");
		break;

	case NOTIFICATION_VP_MOUSE_ENTER:
		godot::UtilityFunctions::print("NOTIFICATION_VP_MOUSE_ENTER");
		break;

	case NOTIFICATION_VP_MOUSE_EXIT:
		godot::UtilityFunctions::print("NOTIFICATION_VP_MOUSE_EXIT");
		break;

	case NOTIFICATION_OS_MEMORY_WARNING:
		godot::UtilityFunctions::print("NOTIFICATION_OS_MEMORY_WARNING");
		break;

	case NOTIFICATION_TRANSLATION_CHANGED:
		godot::UtilityFunctions::print("NOTIFICATION_TRANSLATION_CHANGED");
		break;

	case NOTIFICATION_WM_ABOUT:
		godot::UtilityFunctions::print("NOTIFICATION_WM_ABOUT");
		break;

	case NOTIFICATION_CRASH:
		godot::UtilityFunctions::print("NOTIFICATION_CRASH");
		break;

	case NOTIFICATION_OS_IME_UPDATE:
		godot::UtilityFunctions::print("NOTIFICATION_OS_IME_UPDATE");
		break;

	case NOTIFICATION_APPLICATION_RESUMED:
		godot::UtilityFunctions::print("NOTIFICATION_APPLICATION_RESUMED");
		break;

	case NOTIFICATION_APPLICATION_PAUSED:
		godot::UtilityFunctions::print("NOTIFICATION_APPLICATION_PAUSED");
		break;

	case NOTIFICATION_APPLICATION_FOCUS_IN:
		godot::UtilityFunctions::print("NOTIFICATION_APPLICATION_FOCUS_IN");
		break;

	case NOTIFICATION_APPLICATION_FOCUS_OUT:
		godot::UtilityFunctions::print("NOTIFICATION_APPLICATION_FOCUS_OUT");
		break;

	case NOTIFICATION_TEXT_SERVER_CHANGED:
		godot::UtilityFunctions::print("NOTIFICATION_TEXT_SERVER_CHANGED");
		break;

	case NOTIFICATION_TRANSFORM_CHANGED:
		godot::UtilityFunctions::print("NOTIFICATION_TRANSFORM_CHANGED");
		break;

	case NOTIFICATION_ENTER_WORLD:
		godot::UtilityFunctions::print("NOTIFICATION_ENTER_WORLD");
		break;

	case NOTIFICATION_EXIT_WORLD:
		godot::UtilityFunctions::print("NOTIFICATION_EXIT_WORLD");
		break;

	case NOTIFICATION_VISIBILITY_CHANGED:
		godot::UtilityFunctions::print("NOTIFICATION_VISIBILITY_CHANGED");
		break;

	case NOTIFICATION_LOCAL_TRANSFORM_CHANGED:
		godot::UtilityFunctions::print("NOTIFICATION_LOCAL_TRANSFORM_CHANGED");
		break;

	default:
		godot::UtilityFunctions::print("Unknown notification %d", notification);
	}
}

void SimulationTestNode::_bind_methods()
{

}