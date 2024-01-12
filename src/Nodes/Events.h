#pragma once

#include "Simulation/Event/Event.h"

#include <godot_cpp/variant/string_name.hpp>

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

struct NodePostinitializeEvent : sim::Event
{
    explicit NodePostinitializeEvent() {}
};

struct NodePredeleteEvent : sim::Event
{
    explicit NodePredeleteEvent() {}
};

struct NodeEnterTreeEvent : sim::Event
{
    explicit NodeEnterTreeEvent() {}
};

struct NodeExitTreeEvent : sim::Event
{
    explicit NodeExitTreeEvent() {}
};

struct NodeMovedInParentEvent : sim::Event
{
    explicit NodeMovedInParentEvent() {}
};

struct NodeReadyEvent : sim::Event
{
    explicit NodeReadyEvent() {}
};

struct NodePausedEvent : sim::Event
{
    explicit NodePausedEvent() {}
};

struct NodeUnpausedEvent : sim::Event
{
    explicit NodeUnpausedEvent() {}
};

struct NodePhysicsProcessEvent : sim::Event
{
    explicit NodePhysicsProcessEvent() {}
};

struct NodeProcessEvent : sim::Event
{
    explicit NodeProcessEvent() {}
};

struct NodeParentedEvent : sim::Event
{
    explicit NodeParentedEvent() {}
};

struct NodeUnparentedEvent : sim::Event
{
    explicit NodeUnparentedEvent() {}
};

struct NodeSceneInstantiatedEvent : sim::Event
{
    explicit NodeSceneInstantiatedEvent() {}
};

struct NodeDragBeginEvent : sim::Event
{
    explicit NodeDragBeginEvent() {}
};

struct NodeDragEndEvent : sim::Event
{
    explicit NodeDragEndEvent() {}
};

struct NodePathRenamedEvent : sim::Event
{
    explicit NodePathRenamedEvent() {}
};

struct NodeChildOrderChangedEvent : sim::Event
{
    explicit NodeChildOrderChangedEvent() {}
};

struct NodeInternalProcessEvent : sim::Event
{
    explicit NodeInternalProcessEvent() {}
};

struct NodeInternalPhysicsProcessEvent : sim::Event
{
    explicit NodeInternalPhysicsProcessEvent() {}
};

struct NodePostEnterTreeEvent : sim::Event
{
    explicit NodePostEnterTreeEvent() {}
};

struct NodeDisabledEvent : sim::Event
{
    explicit NodeDisabledEvent() {}
};

struct NodeEnabledEvent : sim::Event
{
    explicit NodeEnabledEvent() {}
};

struct NodeRecacheRequestedEvent : sim::Event
{
    explicit NodeRecacheRequestedEvent() {}
};

struct NodeEditorPreSaveEvent : sim::Event
{
    explicit NodeEditorPreSaveEvent() {}
};

struct NodeEditorPostSaveEvent : sim::Event
{
    explicit NodeEditorPostSaveEvent() {}
};

struct NodeWindowMouseEnterEvent : sim::Event
{
    explicit NodeWindowMouseEnterEvent() {}
};

struct NodeWindowMouseExitEvent : sim::Event
{
    explicit NodeWindowMouseExitEvent() {}
};

struct NodeWindowFocusInEvent : sim::Event
{
    explicit NodeWindowFocusInEvent() {}
};

struct NodeWindowFocusOutEvent : sim::Event
{
    explicit NodeWindowFocusOutEvent() {}
};

struct NodeWindowCloseRequestEvent : sim::Event
{
    explicit NodeWindowCloseRequestEvent() {}
};

struct NodeWindowGoBackRequestEvent : sim::Event
{
    explicit NodeWindowGoBackRequestEvent() {}
};

struct NodeWindowSizeChangedEvent : sim::Event
{
    explicit NodeWindowSizeChangedEvent() {}
};

struct OSDpiChangeEvent : sim::Event
{
    explicit OSDpiChangeEvent() {}
};

struct NodeViewportMouseEnterEvent : sim::Event
{
    explicit NodeViewportMouseEnterEvent() {}
};

struct NodeViewportMouseExitEvent : sim::Event
{
    explicit NodeViewportMouseExitEvent() {}
};

struct OSMemoryWarningEvent : sim::Event
{
    explicit OSMemoryWarningEvent() {}
};

struct GodotTranslationChangedEvent : sim::Event
{
    explicit GodotTranslationChangedEvent() {}
};

struct OSAboutEvent : sim::Event
{
    explicit OSAboutEvent() {}
};

struct OSCrashEvent : sim::Event
{
    explicit OSCrashEvent() {}
};

struct OSIMEUpdateEvent : sim::Event
{
    explicit OSIMEUpdateEvent() {}
};

struct OSApplicationResumedEvent : sim::Event
{
    explicit OSApplicationResumedEvent() {}
};

struct OSApplicationPausedEvent : sim::Event
{
    explicit OSApplicationPausedEvent() {}
};

struct OSApplicationFocusInEvent : sim::Event
{
    explicit OSApplicationFocusInEvent() {}
};

struct OSApplicationFocusOutEvent : sim::Event
{
    explicit OSApplicationFocusOutEvent() {}
};

struct GodotTextServerChangedEvent : sim::Event
{
    explicit GodotTextServerChangedEvent() {}
};



struct GodotActionEvent : sim::Event
{
    explicit GodotActionEvent(const godot::InputEventAction& input) :
        input(input)
    {}

    const godot::InputEventAction& input;
};

struct GodotScreenDragEvent : sim::Event
{
    explicit GodotScreenDragEvent(const godot::InputEventScreenDrag& input) :
        input(input)
    {}

    const godot::InputEventScreenDrag& input;
};

struct GodotScreenTouchEvent : sim::Event
{
    explicit GodotScreenTouchEvent(const godot::InputEventScreenTouch& input) :
        input(input)
    {}

    const godot::InputEventScreenTouch& input;
};

struct GodotMagnifyGestureEvent : sim::Event
{
    explicit GodotMagnifyGestureEvent(const godot::InputEventMagnifyGesture& input) :
        input(input)
    {}

    godot::InputEventMagnifyGesture input;
};

struct GodotPanGestureEvent : sim::Event
{
    explicit GodotPanGestureEvent(const godot::InputEventPanGesture& input) :
        input(input)
    {}

    godot::InputEventPanGesture input;
};

struct GodotKeyEvent : sim::Event
{
    explicit GodotKeyEvent(const godot::InputEventKey& input) :
        input(input)
    {}

    godot::InputEventKey input;
};

struct GodotMouseButtonEvent : sim::Event
{
    explicit GodotMouseButtonEvent(const godot::InputEventMouseButton& input) :
        input(input)
    {}

    godot::InputEventMouseButton input;
};

struct GodotMouseMotionEvent : sim::Event
{
    explicit GodotMouseMotionEvent(const godot::InputEventMouseMotion& input) :
        input(input)
    {}

    godot::InputEventMouseMotion input;
};

struct GodotJoypadButtonEvent : sim::Event
{
    explicit GodotJoypadButtonEvent(const godot::InputEventJoypadButton& input) :
        input(input)
    {}

    godot::InputEventJoypadButton input;
};

struct GodotJoypadMotionEvent : sim::Event
{
    explicit GodotJoypadMotionEvent(const godot::InputEventJoypadMotion& input) :
        input(input)
    {}

    godot::InputEventJoypadMotion input;
};

struct GodotMIDIEvent : sim::Event
{
    explicit GodotMIDIEvent(const godot::InputEventMIDI& input) :
        input(input)
    {}

    godot::InputEventMIDI input;
};

struct GodotShortcutEvent : sim::Event
{
    explicit GodotShortcutEvent(const godot::InputEventShortcut& input) :
        input(input)
    {}

    godot::InputEventShortcut input;
};



struct NodeTransformChangedEvent : sim::Event
{
    explicit NodeTransformChangedEvent() {}
};

struct NodeEnterWorldEvent : sim::Event
{
    explicit NodeEnterWorldEvent() {}
};

struct NodeExitWorldEvent : sim::Event
{
    explicit NodeExitWorldEvent() {}
};

struct NodeVisibilityChangedEvent : sim::Event
{
    explicit NodeVisibilityChangedEvent() {}
};

struct NodeLocalTransformChangedEvent : sim::Event
{
    explicit NodeLocalTransformChangedEvent() {}
};