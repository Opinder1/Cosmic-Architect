#pragma once

#include "Simulation/UUID.h"

#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/node3d.hpp>

namespace sim
{
	class SimulationMessager;
}

class SimulationTestNode : public godot::Node3D
{
	GDCLASS(SimulationTestNode, godot::Node3D);

public:
	SimulationTestNode();

	~SimulationTestNode();

public:
	void _process(double delta) override {}

	void _physics_process(double delta) override {}

	void _input(const godot::Ref<godot::InputEvent>& event) override;

	void _shortcut_input(const godot::Ref<godot::InputEvent>& event) override;

	void _unhandled_input(const godot::Ref<godot::InputEvent>& event) override;

	void _unhandled_key_input(const godot::Ref<godot::InputEvent>& event) override;

private:
	void _notification(int notification);

	static void _bind_methods();

private:
	sim::UUID m_simulation_id;

	sim::SimulationMessager* m_simulation_ptr;
};