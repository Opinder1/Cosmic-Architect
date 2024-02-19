#include "SimulationNode.h"
#include "Events.h"

#include "Simulation/Simulation.h"
#include "Simulation/SimulationServer.h"
#include "Simulation/SimulationBuilder.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/input_event_action.hpp>
#include <godot_cpp/classes/input_event_joypad_button.hpp>
#include <godot_cpp/classes/input_event_joypad_motion.hpp>
#include <godot_cpp/classes/input_event_midi.hpp>
#include <godot_cpp/classes/input_event_shortcut.hpp>
#include <godot_cpp/classes/input_event_screen_drag.hpp>
#include <godot_cpp/classes/input_event_screen_touch.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_magnify_gesture.hpp>
#include <godot_cpp/classes/input_event_pan_gesture.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/world3d.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

SimulationNode::SimulationNode()
{
    set_notify_transform(true);
    set_notify_local_transform(true);
}

SimulationNode::~SimulationNode()
{

}

godot::PackedStringArray SimulationNode::GetAllSimulations()
{
    std::vector<sim::UUID> ids = sim::SimulationServer::GetSingleton()->GetAllSimulations();

    godot::PackedStringArray out;

    for (const sim::UUID& id : ids)
    {
        out.push_back(id.ToGodotString());
    }

    return out;
}

godot::StringName SimulationNode::Create(const godot::String& config_path)
{
    sim::UUID uuid = sim::SimulationServer::GetSingleton()->CreateSimulation(std::make_unique<sim::EmptySimulationBuilder>(), sim::SimulationServer::CreateMethod::Thread);

    return uuid.ToGodotString();
}

void SimulationNode::Delete(const godot::StringName& id)
{
    sim::SimulationServer::GetSingleton()->DeleteSimulation(sim::UUID(id));
}

bool SimulationNode::Start(const godot::StringName& id)
{
    return sim::SimulationServer::GetSingleton()->StartSimulation(sim::UUID(id));
}

void SimulationNode::Stop(const godot::StringName& id)
{
    sim::SimulationServer::GetSingleton()->StopSimulation(sim::UUID(id));
}

bool SimulationNode::IsValid(const godot::StringName& id)
{
    return sim::SimulationServer::GetSingleton()->IsSimulation(sim::UUID(id));
}

bool SimulationNode::IsRunning(const godot::StringName& id)
{
    return sim::SimulationServer::GetSingleton()->IsSimulationRunning(sim::UUID(id)) == sim::SimulationServer::Result::True;
}

bool SimulationNode::IsStopping(const godot::StringName& id)
{
    return sim::SimulationServer::GetSingleton()->IsSimulationStopping(sim::UUID(id)) == sim::SimulationServer::Result::True;
}

void SimulationNode::AttemptFreeMemory()
{
    sim::SimulationServer::GetSingleton()->AttemptFreeMemory();
}

godot::PackedStringArray SimulationNode::GetSimulations()
{
    godot::PackedStringArray out;

    for (auto&& [id, ref] : m_simulations)
    {
        out.push_back(id.ToGodotString());
    }

    return out;
}

bool SimulationNode::Add(const godot::StringName& id)
{
    sim::UUID uuid(id);

    if (m_simulations.find(uuid) != m_simulations.end())
    {
        DEBUG_PRINT_INFO("The simulation has already be added");
        return false;
    }

    // Check that the simulation is running before we try and acquire it with this thread
    switch(sim::SimulationServer::GetSingleton()->IsSimulationRunning(uuid))
    {
    case sim::SimulationServer::Result::True:
        break;

    case sim::SimulationServer::Result::False:
        DEBUG_PRINT_INFO("The requested simulation is not running so can't be added");
        return false;

    case sim::SimulationServer::Result::Invalid:
        DEBUG_PRINT_INFO("The requested simulation doesn't exist so couldn't be added");
        return false;
    }

    SimulationReference ref;

    ref.simulation_id = uuid;

    // Start trying to acquire
    if (!sim::SimulationServer::GetSingleton()->ThreadAcquireSimulation(uuid))
    {
        DEBUG_PRINT_INFO("Failed to acquire simulation");
        return false;
    }

    ref.is_acquiring = true;

    m_simulations.emplace(uuid, std::move(ref));

    return true;
}

bool SimulationNode::Remove(const godot::StringName& id)
{
    sim::UUID uuid(id);

    auto it = m_simulations.find(uuid);

    if (it == m_simulations.end())
    {
        DEBUG_PRINT_INFO("Simulation was never added to this node");
        return false;
    }

    SimulationReference& ref = it->second;

    // Make sure to have the simulation deinitialize its world and tree data
    if (ref.simulation_ptr != nullptr)
    {
        godot::Ref<godot::World3D> world = get_world_3d();

        if (world.is_valid())
        {
            ref.simulation_ptr->PostEvent(NodeExitWorldEvent());
        }

        if (is_inside_tree())
        {
            ref.simulation_ptr->PostEvent(NodeExitTreeEvent());
        }
    }

    // Stop acquiring if we are either in the acquiring process or acquired
    if (ref.is_acquiring || ref.simulation_ptr != nullptr)
    {
        sim::SimulationServer::GetSingleton()->ThreadReleaseSimulation(ref.simulation_id);

        ref.is_acquiring = false;
        ref.simulation_ptr = nullptr;
    }

    m_simulations.erase(it);

    return true;
}

bool SimulationNode::IsAcquiring(const godot::StringName& id)
{
    sim::UUID uuid(id);

    auto it = m_simulations.find(uuid);

    if (it == m_simulations.end())
    {
        return false;
    }

    return it->second.is_acquiring;
}

bool SimulationNode::IsThreadAcquired(const godot::StringName& id)
{
    return GetAcquiredSimulation(id) != nullptr;
}

sim::SimulationMessager* SimulationNode::GetAcquiredSimulation(const godot::StringName& id)
{
    sim::UUID uuid(id);

    auto it = m_simulations.find(uuid);

    if (it == m_simulations.end())
    {
        return nullptr;
    }

    return it->second.simulation_ptr;
}

void SimulationNode::TryGetAcquired(SimulationReference& ref)
{
    DEBUG_ASSERT(ref.is_acquiring, "We should be acquiring");

    ref.simulation_ptr = sim::SimulationServer::GetSingleton()->TryGetAcquiredSimulation(ref.simulation_id);

    if (ref.simulation_ptr == nullptr) // We haven't acquired yet
    {
        return;
    }

    ref.is_acquiring = false;

    // Make sure to tell the simulation if we are already in the tree and world
    if (is_inside_tree())
    {
        ref.simulation_ptr->PostEvent(NodeEnterTreeEvent(*get_tree()));
    }

    godot::Ref<godot::World3D> world = get_world_3d();

    if (world.is_valid())
    {
        ref.simulation_ptr->PostEvent(NodeEnterWorldEvent(*world));

        ref.simulation_ptr->PostEvent(NodeTransformChangedEvent(get_global_transform()));

        ref.simulation_ptr->PostEvent(NodeLocalTransformChangedEvent(get_transform()));
    }
}

template<class EventT>
void SimulationNode::SendEventToAll(const EventT& event)
{
    for (auto&& [id, ref] : m_simulations)
    {
        if (ref.simulation_ptr != nullptr)
        {
            ref.simulation_ptr->PostEvent(event);
        }
    }
}

void SimulationNode::ManualTickAll()
{
    for (auto&& [id, ref] : m_simulations)
    {
        if (ref.is_acquiring)
        {
            TryGetAcquired(ref);
        }
    }

    for (auto&& [id, ref] : m_simulations)
    {
        if (ref.simulation_ptr != nullptr)
        {
            ref.simulation_ptr->ManualTick();
            ref.simulation_ptr->PostEvent(NodePhysicsProcessEvent());
        }
    }
}

void SimulationNode::_input(const godot::Ref<godot::InputEvent>& event)
{
    if (godot::InputEventAction* action = godot::Object::cast_to<godot::InputEventAction>(event.ptr()))
    {
        SendEventToAll(GodotActionEvent(*action));
    }
    else if (godot::Object::cast_to<godot::InputEventFromWindow>(event.ptr()))
    {
        if (godot::InputEventScreenDrag* screen_drag = godot::Object::cast_to<godot::InputEventScreenDrag>(event.ptr()))
        {
            SendEventToAll(GodotScreenDragEvent(*screen_drag));
        }
        else if (godot::InputEventScreenTouch* screen_touch = godot::Object::cast_to<godot::InputEventScreenTouch>(event.ptr()))
        {
            SendEventToAll(GodotScreenTouchEvent(*screen_touch));
        }
        else if (godot::Object::cast_to<godot::InputEventWithModifiers>(event.ptr()))
        {
            if (godot::Object::cast_to<godot::InputEventGesture>(event.ptr()))
            {
                if (godot::InputEventMagnifyGesture* magnify_gesture = godot::Object::cast_to<godot::InputEventMagnifyGesture>(event.ptr()))
                {
                    SendEventToAll(GodotMagnifyGestureEvent(*magnify_gesture));
                }
                else if (godot::InputEventPanGesture* pan_gesture = godot::Object::cast_to<godot::InputEventPanGesture>(event.ptr()))
                {
                    SendEventToAll(GodotPanGestureEvent(*pan_gesture));
                }
            }
            else if (godot::InputEventKey* key = godot::Object::cast_to<godot::InputEventKey>(event.ptr()))
            {
                SendEventToAll(GodotKeyEvent(*key));
            }
            else if (godot::Object::cast_to<godot::InputEventMouse>(event.ptr()))
            {
                if (godot::InputEventMouseButton* mouse_button = godot::Object::cast_to<godot::InputEventMouseButton>(event.ptr()))
                {
                    SendEventToAll(GodotMouseButtonEvent(*mouse_button));
                }
                else  if (godot::InputEventMouseMotion* mouse_motion = godot::Object::cast_to<godot::InputEventMouseMotion>(event.ptr()))
                {
                    SendEventToAll(GodotMouseMotionEvent(*mouse_motion));
                }
            }
        }
    }
    else if (godot::InputEventJoypadButton* joypad_button = godot::Object::cast_to<godot::InputEventJoypadButton>(event.ptr()))
    {
        SendEventToAll(GodotJoypadButtonEvent(*joypad_button));
    }
    else if (godot::InputEventJoypadMotion* joypad_motion = godot::Object::cast_to<godot::InputEventJoypadMotion>(event.ptr()))
    {
        SendEventToAll(GodotJoypadMotionEvent(*joypad_motion));
    }
    else if (godot::InputEventMIDI* midi = godot::Object::cast_to<godot::InputEventMIDI>(event.ptr()))
    {
        SendEventToAll(GodotMIDIEvent(*midi));
    }
    else if (godot::InputEventShortcut* shortcut = godot::Object::cast_to<godot::InputEventShortcut>(event.ptr()))
    {
        SendEventToAll(GodotShortcutEvent(*shortcut));
    }
}

void SimulationNode::_notification(int notification)
{
	switch (notification)
	{
    case NOTIFICATION_POSTINITIALIZE:
        SendEventToAll(NodePostinitializeEvent());
        break;

    case NOTIFICATION_PREDELETE:
        SendEventToAll(NodePredeleteEvent());
        break;

    case NOTIFICATION_ENTER_TREE:
        SendEventToAll(NodeEnterTreeEvent(*get_tree()));
        break;

    case NOTIFICATION_EXIT_TREE:
        SendEventToAll(NodeExitTreeEvent());
        break;

    case NOTIFICATION_MOVED_IN_PARENT:
        SendEventToAll(NodeMovedInParentEvent());
        break;

    case NOTIFICATION_READY:
        SendEventToAll(NodeReadyEvent());
        break;

    case NOTIFICATION_PAUSED:
        SendEventToAll(NodePausedEvent());
        break;

    case NOTIFICATION_UNPAUSED:
        SendEventToAll(NodeUnpausedEvent());
        break;

    case NOTIFICATION_PHYSICS_PROCESS:
        ManualTickAll();
        break;

    case NOTIFICATION_PROCESS:
        SendEventToAll(NodeProcessEvent());
        break;

    case NOTIFICATION_PARENTED:
        SendEventToAll(NodeParentedEvent(*get_parent()));
        break;

    case NOTIFICATION_UNPARENTED:
        SendEventToAll(NodeUnparentedEvent());
        break;

    case NOTIFICATION_SCENE_INSTANTIATED:
        SendEventToAll(NodeSceneInstantiatedEvent(get_scene_file_path()));
        break;

    case NOTIFICATION_DRAG_BEGIN:
        SendEventToAll(NodeDragBeginEvent());
        break;

    case NOTIFICATION_DRAG_END:
        SendEventToAll(NodeDragEndEvent());
        break;

    case NOTIFICATION_PATH_RENAMED:
        SendEventToAll(NodePathRenamedEvent(get_path()));
        break;

    case NOTIFICATION_CHILD_ORDER_CHANGED:
        SendEventToAll(NodeChildOrderChangedEvent());
        break;

    case NOTIFICATION_INTERNAL_PROCESS:
        SendEventToAll(NodeInternalProcessEvent());
        break;

    case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
        SendEventToAll(NodeInternalPhysicsProcessEvent());
        break;

    case NOTIFICATION_POST_ENTER_TREE:
        SendEventToAll(NodePostEnterTreeEvent());
        break;

    case NOTIFICATION_DISABLED:
        SendEventToAll(NodeDisabledEvent());
        break;

    case NOTIFICATION_ENABLED:
        SendEventToAll(NodeEnabledEvent());
        break;

    case NOTIFICATION_NODE_RECACHE_REQUESTED:
        SendEventToAll(NodeRecacheRequestedEvent());
        break;

    case NOTIFICATION_EDITOR_PRE_SAVE:
        SendEventToAll(NodeEditorPreSaveEvent());
        break;

    case NOTIFICATION_EDITOR_POST_SAVE:
        SendEventToAll(NodeEditorPostSaveEvent());
        break;

    case NOTIFICATION_WM_MOUSE_ENTER:
        SendEventToAll(NodeWindowMouseEnterEvent());
        break;

    case NOTIFICATION_WM_MOUSE_EXIT:
        SendEventToAll(NodeWindowMouseExitEvent());
        break;

    case NOTIFICATION_WM_WINDOW_FOCUS_IN:
        SendEventToAll(NodeWindowFocusInEvent());
        break;

    case NOTIFICATION_WM_WINDOW_FOCUS_OUT:
        SendEventToAll(NodeWindowFocusOutEvent());
        break;

    case NOTIFICATION_WM_CLOSE_REQUEST:
        SendEventToAll(NodeWindowCloseRequestEvent());
        break;

    case NOTIFICATION_WM_GO_BACK_REQUEST:
        SendEventToAll(NodeWindowGoBackRequestEvent());
        break;

    case NOTIFICATION_WM_SIZE_CHANGED:
        SendEventToAll(NodeWindowSizeChangedEvent());
        break;

    case NOTIFICATION_WM_DPI_CHANGE:
        SendEventToAll(OSDpiChangeEvent());
        break;

    case NOTIFICATION_VP_MOUSE_ENTER:
        SendEventToAll(NodeViewportMouseEnterEvent());
        break;

    case NOTIFICATION_VP_MOUSE_EXIT:
        SendEventToAll(NodeViewportMouseExitEvent());
        break;

    case NOTIFICATION_OS_MEMORY_WARNING:
        SendEventToAll(OSMemoryWarningEvent());
        break;

    case NOTIFICATION_TRANSLATION_CHANGED:
        SendEventToAll(GodotTranslationChangedEvent());
        break;

    case NOTIFICATION_WM_ABOUT:
        SendEventToAll(OSAboutEvent());
        break;

    case NOTIFICATION_CRASH:
        SendEventToAll(OSCrashEvent());
        break;

    case NOTIFICATION_OS_IME_UPDATE:
        SendEventToAll(OSIMEUpdateEvent());
        break;

    case NOTIFICATION_APPLICATION_RESUMED:
        SendEventToAll(OSApplicationResumedEvent());
        break;

    case NOTIFICATION_APPLICATION_PAUSED:
        SendEventToAll(OSApplicationPausedEvent());
        break;

    case NOTIFICATION_APPLICATION_FOCUS_IN:
        SendEventToAll(OSApplicationFocusInEvent());
        break;

    case NOTIFICATION_APPLICATION_FOCUS_OUT:
        SendEventToAll(OSApplicationFocusOutEvent());
        break;

    case NOTIFICATION_TEXT_SERVER_CHANGED:
        SendEventToAll(GodotTextServerChangedEvent());
        break;

    case NOTIFICATION_TRANSFORM_CHANGED:
        SendEventToAll(NodeTransformChangedEvent(get_global_transform()));
        break;

    case NOTIFICATION_ENTER_WORLD:
        SendEventToAll(NodeEnterWorldEvent(get_world_3d()));
        break;

    case NOTIFICATION_EXIT_WORLD:
        SendEventToAll(NodeExitWorldEvent());
        break;

    case NOTIFICATION_VISIBILITY_CHANGED:
        SendEventToAll(NodeVisibilityChangedEvent(is_visible_in_tree()));
        break;

    case NOTIFICATION_LOCAL_TRANSFORM_CHANGED:
        SendEventToAll(NodeLocalTransformChangedEvent(get_transform()));
        break;

	default:
		godot::UtilityFunctions::print("Unknown notification: ", notification);
        break;
	}
}

void SimulationNode::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("get_simulations"), &SimulationNode::GetSimulations);
    godot::ClassDB::bind_method(godot::D_METHOD("add", "id"), &SimulationNode::Add);
    godot::ClassDB::bind_method(godot::D_METHOD("remove", "id"), &SimulationNode::Remove);
    godot::ClassDB::bind_method(godot::D_METHOD("is_acquiring", "id"), &SimulationNode::IsAcquiring);
    godot::ClassDB::bind_method(godot::D_METHOD("is_thread_acquired", "id"), &SimulationNode::IsThreadAcquired);

    godot::ClassDB::bind_static_method("SimulationNode", godot::D_METHOD("get_all_simulations"), &SimulationNode::AttemptFreeMemory);
    godot::ClassDB::bind_static_method("SimulationNode", godot::D_METHOD("create", "id"), &SimulationNode::Create);
    godot::ClassDB::bind_static_method("SimulationNode", godot::D_METHOD("delete", "id"), &SimulationNode::Delete);
    godot::ClassDB::bind_static_method("SimulationNode", godot::D_METHOD("start", "id"), &SimulationNode::Start);
    godot::ClassDB::bind_static_method("SimulationNode", godot::D_METHOD("stop", "id"), &SimulationNode::Stop);
    godot::ClassDB::bind_static_method("SimulationNode", godot::D_METHOD("is_valid", "id"), &SimulationNode::IsValid);
    godot::ClassDB::bind_static_method("SimulationNode", godot::D_METHOD("is_running", "id"), &SimulationNode::IsRunning);
    godot::ClassDB::bind_static_method("SimulationNode", godot::D_METHOD("is_stopping", "id"), &SimulationNode::IsStopping);
    godot::ClassDB::bind_static_method("SimulationNode", godot::D_METHOD("attempt_free_memory"), &SimulationNode::AttemptFreeMemory);
}