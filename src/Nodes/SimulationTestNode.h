#pragma once

#include "SimulationNode.h"

class SimulationTestNode : public SimulationNode
{
	GDCLASS(SimulationTestNode, SimulationNode);

public:
	SimulationTestNode();

	~SimulationTestNode();

public:
	static void _bind_methods();
};