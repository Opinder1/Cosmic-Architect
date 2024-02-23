#pragma once

#include "Util/Callback.h"

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/dir_access.hpp>

namespace sim
{
	struct Simulation;

	// An interface for defining a method for constructing a simulation and adding systems and globals to it
	class SimulationBuilder
	{
	public:
		using SimulationApplicator = cb::Callback<void(Simulation&)>;

	public:
		SimulationBuilder();

		// Our build function that will be called once
		virtual void Build(Simulation& simulation) const = 0;

	protected:
		static void AddDefaultSystems(Simulation& simulation);

		// Add a system to the simulation
		template<class SystemT>
		static void AddSystem(Simulation& simulation)
		{
			AddSystem(simulation, SystemT::OnInitialize, SystemT::OnShutdown);
		}

		// Add a system using an emmiter callback
		static void AddSystem(Simulation& simulation, const SimulationApplicator& initialize, const SimulationApplicator& shutdown);
	};

	class EmptySimulationBuilder : public SimulationBuilder
	{
	public:
		EmptySimulationBuilder();

		void Build(Simulation& simulation) const final;
	};

	class DirectorySimulationBuilder : protected SimulationBuilder
	{
	public:
		DirectorySimulationBuilder(const godot::String& path);

		// Build with a given directory
		virtual void Build(Simulation& simulation, const godot::DirAccess& directory) const = 0;

		void Build(Simulation& simulation) const final;

	private:
		godot::String m_path;
	};
}