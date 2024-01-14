#pragma once

#include "Simulation/UUID.h"

#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/node3d.hpp>

namespace sim
{
	class SimulationMessager;
}

class SimulationNode : public godot::Node3D
{
	GDCLASS(SimulationNode, godot::Node3D);

public:
	SimulationNode();

	~SimulationNode();

	sim::UUID GetID();

	void Start();

	void Stop();

	void ThreadAcquire();

	void ThreadRelease();

	bool IsValid();

	bool IsRunning();

	bool IsStopping();

	bool IsAcquiring();

	bool IsThreadAcquired();

	static void AttemptFreeMemory();

protected:
	sim::SimulationMessager* GetAcquiredSimulation();

private:
	bool UpdateManuallyTicked();

public:
	void _process(double delta) override {}

	void _physics_process(double delta) override {}

	void _input(const godot::Ref<godot::InputEvent>& event) override;

	void _notification(int notification);

	static void _bind_methods();

private:
	sim::UUID m_simulation_id;

	bool m_is_acquiring;
	sim::SimulationMessager* m_simulation_ptr;
};