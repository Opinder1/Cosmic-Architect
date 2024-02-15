#pragma once

#include "SimulationNode.h"

#include <godot_cpp/variant/string_name.hpp>

class SimulationTestNode : public SimulationNode
{
	GDCLASS(SimulationTestNode, SimulationNode);

public:
	SimulationTestNode();

	~SimulationTestNode();

public:
	void _notification(int notification);

	static void _bind_methods();

private:
	godot::StringName m_id;
	bool m_try_and_add;
	bool m_added;
};