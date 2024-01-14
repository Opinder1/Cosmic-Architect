#include "SimulationNode.h"
#include "Events.h"

#include "Simulation/Simulation.h"
#include "Simulation/SimulationServer.h"

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

#include <godot_cpp/variant/utility_functions.hpp>

SimulationNode::SimulationNode() :
	m_simulation_ptr(nullptr),
    m_is_acquiring(false)
{
	set_notify_transform(true);
	set_notify_local_transform(true);

	m_simulation_id = sim::SimulationServer::GetSingleton()->CreateSimulation(60, true);
}

SimulationNode::~SimulationNode()
{
	sim::SimulationServer::GetSingleton()->DeleteSimulation(m_simulation_id);
}

sim::UUID SimulationNode::GetID()
{
    return m_simulation_id;
}

void SimulationNode::Start()
{
    sim::SimulationServer::GetSingleton()->StartSimulation(m_simulation_id);
}

void SimulationNode::Stop()
{
    sim::SimulationServer::GetSingleton()->StopSimulation(m_simulation_id);
}

void SimulationNode::ThreadAcquire()
{
    if (sim::SimulationServer::GetSingleton()->ThreadAcquireSimulation(m_simulation_id))
    {
        m_is_acquiring = true;
    }
}

void SimulationNode::ThreadRelease()
{
    if (IsAcquiring() || IsThreadAcquired())
    {
        sim::SimulationServer::GetSingleton()->ThreadReleaseSimulation(m_simulation_id);

        m_is_acquiring = false;
        m_simulation_ptr = nullptr;
    }
}

bool SimulationNode::IsValid()
{
    return sim::SimulationServer::GetSingleton()->IsSimulation(m_simulation_id);
}

bool SimulationNode::IsRunning()
{
    return sim::SimulationServer::GetSingleton()->IsSimulationRunning(m_simulation_id) == sim::SimulationServer::Result::True;
}

bool SimulationNode::IsStopping()
{
    return sim::SimulationServer::GetSingleton()->IsSimulationStopping(m_simulation_id) == sim::SimulationServer::Result::True;
}

bool SimulationNode::IsAcquiring()
{
    return m_is_acquiring;
}

bool SimulationNode::IsThreadAcquired()
{
    return m_simulation_ptr != nullptr && m_simulation_ptr->IsExternallyTicked();
}

sim::SimulationMessager* SimulationNode::GetAcquiredSimulation()
{
    return m_simulation_ptr;
}

void SimulationNode::AttemptFreeMemory()
{
    sim::SimulationServer::GetSingleton()->AttemptFreeMemory();
}

bool SimulationNode::UpdateManuallyTicked()
{
    if (IsThreadAcquired())
    {
        return true;
    }
    
    if (m_is_acquiring) // If we are acquired then try and get the simulation pointer
    {
        m_simulation_ptr = sim::SimulationServer::GetSingleton()->TryGetAcquiredSimulation(m_simulation_id);
        m_is_acquiring = false;

        return IsThreadAcquired();
    }
    
    return false;
}

void SimulationNode::_input(const godot::Ref<godot::InputEvent>& event)
{
    if (!UpdateManuallyTicked())
    {
        return;
    }

    if (godot::InputEventAction* action = godot::Object::cast_to<godot::InputEventAction>(event.ptr()))
    {
        m_simulation_ptr->PostEvent(GodotActionEvent(*action));
    }
    else if (godot::Object::cast_to<godot::InputEventFromWindow>(event.ptr()))
    {
        if (godot::InputEventScreenDrag* screen_drag = godot::Object::cast_to<godot::InputEventScreenDrag>(event.ptr()))
        {
            m_simulation_ptr->PostEvent(GodotScreenDragEvent(*screen_drag));
        }
        else if (godot::InputEventScreenTouch* screen_touch = godot::Object::cast_to<godot::InputEventScreenTouch>(event.ptr()))
        {
            m_simulation_ptr->PostEvent(GodotScreenTouchEvent(*screen_touch));
        }
        else if (godot::Object::cast_to<godot::InputEventWithModifiers>(event.ptr()))
        {
            if (godot::Object::cast_to<godot::InputEventGesture>(event.ptr()))
            {
                if (godot::InputEventMagnifyGesture* magnify_gesture = godot::Object::cast_to<godot::InputEventMagnifyGesture>(event.ptr()))
                {
                    m_simulation_ptr->PostEvent(GodotMagnifyGestureEvent(*magnify_gesture));
                }
                else if (godot::InputEventPanGesture* pan_gesture = godot::Object::cast_to<godot::InputEventPanGesture>(event.ptr()))
                {
                    m_simulation_ptr->PostEvent(GodotPanGestureEvent(*pan_gesture));
                }
            }
            else if (godot::InputEventKey* key = godot::Object::cast_to<godot::InputEventKey>(event.ptr()))
            {
                m_simulation_ptr->PostEvent(GodotKeyEvent(*key));
            }
            else if (godot::Object::cast_to<godot::InputEventMouse>(event.ptr()))
            {
                if (godot::InputEventMouseButton* mouse_button = godot::Object::cast_to<godot::InputEventMouseButton>(event.ptr()))
                {
                    m_simulation_ptr->PostEvent(GodotMouseButtonEvent(*mouse_button));
                }
                else  if (godot::InputEventMouseMotion* mouse_motion = godot::Object::cast_to<godot::InputEventMouseMotion>(event.ptr()))
                {
                    m_simulation_ptr->PostEvent(GodotMouseMotionEvent(*mouse_motion));
                }
            }
        }
    }
    else if (godot::InputEventJoypadButton* joypad_button = godot::Object::cast_to<godot::InputEventJoypadButton>(event.ptr()))
    {
        m_simulation_ptr->PostEvent(GodotJoypadButtonEvent(*joypad_button));
    }
    else if (godot::InputEventJoypadMotion* joypad_motion = godot::Object::cast_to<godot::InputEventJoypadMotion>(event.ptr()))
    {
        m_simulation_ptr->PostEvent(GodotJoypadMotionEvent(*joypad_motion));
    }
    else if (godot::InputEventMIDI* midi = godot::Object::cast_to<godot::InputEventMIDI>(event.ptr()))
    {
        m_simulation_ptr->PostEvent(GodotMIDIEvent(*midi));
    }
    else if (godot::InputEventShortcut* shortcut = godot::Object::cast_to<godot::InputEventShortcut>(event.ptr()))
    {
        m_simulation_ptr->PostEvent(GodotShortcutEvent(*shortcut));
    }
}

void SimulationNode::_notification(int notification)
{
    if (!UpdateManuallyTicked())
    {
        return;
    }

	switch (notification)
	{
    case NOTIFICATION_POSTINITIALIZE:
        m_simulation_ptr->PostEvent(NodePostinitializeEvent());
        break;

    case NOTIFICATION_PREDELETE:
        m_simulation_ptr->PostEvent(NodePredeleteEvent());
        break;

    case NOTIFICATION_ENTER_TREE:
        m_simulation_ptr->PostEvent(NodeEnterTreeEvent());
        break;

    case NOTIFICATION_EXIT_TREE:
        m_simulation_ptr->PostEvent(NodeExitTreeEvent());
        break;

    case NOTIFICATION_MOVED_IN_PARENT:
        m_simulation_ptr->PostEvent(NodeMovedInParentEvent());
        break;

    case NOTIFICATION_READY:
        m_simulation_ptr->PostEvent(NodeReadyEvent());
        break;

    case NOTIFICATION_PAUSED:
        m_simulation_ptr->PostEvent(NodePausedEvent());
        break;

    case NOTIFICATION_UNPAUSED:
        m_simulation_ptr->PostEvent(NodeUnpausedEvent());
        break;

    case NOTIFICATION_PHYSICS_PROCESS:
        m_simulation_ptr->ManualTick();
        m_simulation_ptr->PostEvent(NodePhysicsProcessEvent());
        break;

    case NOTIFICATION_PROCESS:
        m_simulation_ptr->PostEvent(NodeProcessEvent());
        break;

    case NOTIFICATION_PARENTED:
        m_simulation_ptr->PostEvent(NodeParentedEvent());
        break;

    case NOTIFICATION_UNPARENTED:
        m_simulation_ptr->PostEvent(NodeUnparentedEvent());
        break;

    case NOTIFICATION_SCENE_INSTANTIATED:
        m_simulation_ptr->PostEvent(NodeSceneInstantiatedEvent());
        break;

    case NOTIFICATION_DRAG_BEGIN:
        m_simulation_ptr->PostEvent(NodeDragBeginEvent());
        break;

    case NOTIFICATION_DRAG_END:
        m_simulation_ptr->PostEvent(NodeDragEndEvent());
        break;

    case NOTIFICATION_PATH_RENAMED:
        m_simulation_ptr->PostEvent(NodePathRenamedEvent());
        break;

    case NOTIFICATION_CHILD_ORDER_CHANGED:
        m_simulation_ptr->PostEvent(NodeChildOrderChangedEvent());
        break;

    case NOTIFICATION_INTERNAL_PROCESS:
        m_simulation_ptr->PostEvent(NodeInternalProcessEvent());
        break;

    case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
        m_simulation_ptr->PostEvent(NodeInternalPhysicsProcessEvent());
        break;

    case NOTIFICATION_POST_ENTER_TREE:
        m_simulation_ptr->PostEvent(NodePostEnterTreeEvent());
        break;

    case NOTIFICATION_DISABLED:
        m_simulation_ptr->PostEvent(NodeDisabledEvent());
        break;

    case NOTIFICATION_ENABLED:
        m_simulation_ptr->PostEvent(NodeEnabledEvent());
        break;

    case NOTIFICATION_NODE_RECACHE_REQUESTED:
        m_simulation_ptr->PostEvent(NodeRecacheRequestedEvent());
        break;

    case NOTIFICATION_EDITOR_PRE_SAVE:
        m_simulation_ptr->PostEvent(NodeEditorPreSaveEvent());
        break;

    case NOTIFICATION_EDITOR_POST_SAVE:
        m_simulation_ptr->PostEvent(NodeEditorPostSaveEvent());
        break;

    case NOTIFICATION_WM_MOUSE_ENTER:
        m_simulation_ptr->PostEvent(NodeWindowMouseEnterEvent());
        break;

    case NOTIFICATION_WM_MOUSE_EXIT:
        m_simulation_ptr->PostEvent(NodeWindowMouseExitEvent());
        break;

    case NOTIFICATION_WM_WINDOW_FOCUS_IN:
        m_simulation_ptr->PostEvent(NodeWindowFocusInEvent());
        break;

    case NOTIFICATION_WM_WINDOW_FOCUS_OUT:
        m_simulation_ptr->PostEvent(NodeWindowFocusOutEvent());
        break;

    case NOTIFICATION_WM_CLOSE_REQUEST:
        m_simulation_ptr->PostEvent(NodeWindowCloseRequestEvent());
        break;

    case NOTIFICATION_WM_GO_BACK_REQUEST:
        m_simulation_ptr->PostEvent(NodeWindowGoBackRequestEvent());
        break;

    case NOTIFICATION_WM_SIZE_CHANGED:
        m_simulation_ptr->PostEvent(NodeWindowSizeChangedEvent());
        break;

    case NOTIFICATION_WM_DPI_CHANGE:
        m_simulation_ptr->PostEvent(OSDpiChangeEvent());
        break;

    case NOTIFICATION_VP_MOUSE_ENTER:
        m_simulation_ptr->PostEvent(NodeViewportMouseEnterEvent());
        break;

    case NOTIFICATION_VP_MOUSE_EXIT:
        m_simulation_ptr->PostEvent(NodeViewportMouseExitEvent());
        break;

    case NOTIFICATION_OS_MEMORY_WARNING:
        m_simulation_ptr->PostEvent(OSMemoryWarningEvent());
        break;

    case NOTIFICATION_TRANSLATION_CHANGED:
        m_simulation_ptr->PostEvent(GodotTranslationChangedEvent());
        break;

    case NOTIFICATION_WM_ABOUT:
        m_simulation_ptr->PostEvent(OSAboutEvent());
        break;

    case NOTIFICATION_CRASH:
        m_simulation_ptr->PostEvent(OSCrashEvent());
        break;

    case NOTIFICATION_OS_IME_UPDATE:
        m_simulation_ptr->PostEvent(OSIMEUpdateEvent());
        break;

    case NOTIFICATION_APPLICATION_RESUMED:
        m_simulation_ptr->PostEvent(OSApplicationResumedEvent());
        break;

    case NOTIFICATION_APPLICATION_PAUSED:
        m_simulation_ptr->PostEvent(OSApplicationPausedEvent());
        break;

    case NOTIFICATION_APPLICATION_FOCUS_IN:
        m_simulation_ptr->PostEvent(OSApplicationFocusInEvent());
        break;

    case NOTIFICATION_APPLICATION_FOCUS_OUT:
        m_simulation_ptr->PostEvent(OSApplicationFocusOutEvent());
        break;

    case NOTIFICATION_TEXT_SERVER_CHANGED:
        m_simulation_ptr->PostEvent(GodotTextServerChangedEvent());
        break;

    case NOTIFICATION_TRANSFORM_CHANGED:
        m_simulation_ptr->PostEvent(NodeTransformChangedEvent());
        break;

    case NOTIFICATION_ENTER_WORLD:
        m_simulation_ptr->PostEvent(NodeEnterWorldEvent());
        break;

    case NOTIFICATION_EXIT_WORLD:
        m_simulation_ptr->PostEvent(NodeExitWorldEvent());
        break;

    case NOTIFICATION_VISIBILITY_CHANGED:
        m_simulation_ptr->PostEvent(NodeVisibilityChangedEvent());
        break;

    case NOTIFICATION_LOCAL_TRANSFORM_CHANGED:
        m_simulation_ptr->PostEvent(NodeLocalTransformChangedEvent());
        break;

	default:
		godot::UtilityFunctions::print("Unknown notification %d", notification);
        break;
	}
}

void SimulationNode::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("start"), &SimulationNode::Start);
    godot::ClassDB::bind_method(godot::D_METHOD("stop"), &SimulationNode::Stop);
    godot::ClassDB::bind_method(godot::D_METHOD("threadacquire"), &SimulationNode::ThreadAcquire);
    godot::ClassDB::bind_method(godot::D_METHOD("threadrelease"), &SimulationNode::ThreadRelease);
    godot::ClassDB::bind_method(godot::D_METHOD("is_valid"), &SimulationNode::IsValid);
    godot::ClassDB::bind_method(godot::D_METHOD("is_acquiring"), &SimulationNode::IsAcquiring);
    godot::ClassDB::bind_method(godot::D_METHOD("is_thread_acquired"), &SimulationNode::IsThreadAcquired);
    godot::ClassDB::bind_method(godot::D_METHOD("is_running"), &SimulationNode::IsRunning);
    godot::ClassDB::bind_method(godot::D_METHOD("is_stopping"), &SimulationNode::IsStopping);

    godot::ClassDB::bind_static_method("SimulationNode", godot::D_METHOD("attempt_free_memory"), &SimulationNode::AttemptFreeMemory);
}