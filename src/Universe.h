#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

#include <robin_hood/robin_hood.h>

#include <optional>

namespace voxel_game
{
	class UniverseSimulation;
}

namespace std
{
	template<>
	struct hash<godot::Ref<voxel_game::UniverseSimulation>>
	{
		size_t operator()(const godot::Ref<voxel_game::UniverseSimulation>& simulation) const;
	};
}

namespace voxel_game
{
	// Main voxel game info class that spawns galaxy simulations
	class Universe : public godot::RefCounted
	{
		GDCLASS(Universe, godot::RefCounted);

	public:
		struct SignalStrings;

		static std::optional<const SignalStrings> k_signals;

	public:
		Universe();
		~Universe();

		godot::Dictionary GetUniverseInfo();
		void ConnectToGalaxyList(const godot::String& ip);
		void DisconnectFromGalaxyList();
		void QueryGalaxyList(const godot::Dictionary& query);
		void PingGalaxy(const godot::String& ip);

		godot::Ref<UniverseSimulation> InitializeLocalGalaxy(const godot::String& galaxy_path);
		godot::Ref<UniverseSimulation> InitializeLocalFragment(const godot::String& fragment_path, const godot::String& fragment_type);
		godot::Ref<UniverseSimulation> InitializeRemoteGalaxy(const godot::String& galaxy_path);

		void Uninitialize(const godot::Ref<UniverseSimulation>& simulation);

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		void OnSimulationStateChanged(uint64_t load_state, const godot::Ref<UniverseSimulation>& simulation);

		void OnSimulationUninitialized(const godot::Ref<UniverseSimulation>& simulation);

	private:
		godot::Dictionary m_universe_info_cache;

		robin_hood::unordered_set<godot::Ref<UniverseSimulation>> m_simulations;
	};

	struct Universe::SignalStrings
	{
		SignalStrings();

		godot::StringName connected_to_galaxy_list;
		godot::StringName disconnected_from_galaxy_list;
		godot::StringName galaxy_list_query_response;
		godot::StringName galaxy_ping_response;
		godot::StringName galaxy_simulation_started;
		godot::StringName galaxy_simulation_stopped;
	};
}