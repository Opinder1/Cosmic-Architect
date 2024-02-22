#pragma once

#include "Util/Callback.h"

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/dir_access.hpp>

namespace sim
{
	struct Simulation;

	class SimulationBuilder
	{
	public:
		using SimulationApplicator = cb::Callback<void(Simulation&)>;

	public:
		SimulationBuilder();

		virtual void Build(Simulation& simulation) const = 0;

	protected:
		void AddDefaultSystems(Simulation& simulation) const;

		// Add a system to a simulation
		template<class SystemT>
		void AddSystem(Simulation& simulation) const
		{
			AddSystem(simulation, SystemT::OnInitialize, SystemT::OnShutdown);
		}

		// Add a system using an emmiter callback
		void AddSystem(Simulation& simulation, const SimulationApplicator& initialize, const SimulationApplicator& shutdown) const;
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

		virtual void Build(Simulation& simulation, const godot::DirAccess& directory) const = 0;

		void Build(Simulation& simulation) const final;

	private:
		godot::String m_path;
	};
}