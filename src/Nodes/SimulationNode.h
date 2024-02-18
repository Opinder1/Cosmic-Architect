#pragma once

#include "Simulation/UUID.h"

#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace sim
{
	class SimulationMessager;
}

class SimulationNode : public godot::Node3D
{
	GDCLASS(SimulationNode, godot::Node3D);

	struct SimulationReference
	{
		sim::UUID simulation_id;
		sim::SimulationMessager* simulation_ptr = nullptr;
		bool is_acquiring = false;
	};

public:
	SimulationNode();
	~SimulationNode();

	godot::PackedStringArray GetSimulations();
	
	// Add a simulation to this node to be acquired
	bool Add(const godot::StringName& id);

	// Remove a simulation from this node to be unacquired
	bool Remove(const godot::StringName& id);

	// Check if we are trying to acquire the simulation
	bool IsAcquiring(const godot::StringName& id);

	// Check if we have acquired the simulation
	bool IsThreadAcquired(const godot::StringName& id);

public:
	// Get the ids of all the current simulations
	static godot::PackedStringArray GetAllSimulations();

	// Create a new simulation loading the specified config path
	static godot::StringName Create(const godot::String& config_path);

	// Try and delete a simulation
	static void Delete(const godot::StringName& id);

	// Try and start a simulation
	static bool Start(const godot::StringName& id);

	// Try and stop a simulation
	static void Stop(const godot::StringName& id);
	
	// Check if a simulation is valid
	static bool IsValid(const godot::StringName& id);

	// Check if a simulation is running
	static bool IsRunning(const godot::StringName& id);

	// Check if a simulation is stopping
	static bool IsStopping(const godot::StringName& id);

	// Attempt to free the memory of all simulations
	static void AttemptFreeMemory();

protected:
	sim::SimulationMessager* GetAcquiredSimulation(const godot::StringName& id);

private:
	// Try and get the simulation that we want to acquire
	void TryGetAcquired(SimulationReference& ref);

	// Send an event to all the simulations acquired by this node
	template<class EventT>
	void SendEventToAll(const EventT& event);

	// Manually tick all the simulations acquired by this node
	void ManualTickAll();

public:
	// Godot node input
	void _input(const godot::Ref<godot::InputEvent>& event) override;

	// Godot object notification
	void _notification(int notification);

	// Godot object bind methods
	static void _bind_methods();

private:
	robin_hood::unordered_flat_map<sim::UUID, SimulationReference> m_simulations;
};