#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

namespace voxel_game
{
	class UniverseSimulation;

	class Universe : public godot::RefCounted
	{
		GDCLASS(Universe, godot::RefCounted);

	public:
		struct Signals;

		static std::unique_ptr<const Signals> k_signals;

	public:
		Universe();
		~Universe();

		// ####### Universe #######

		godot::Dictionary GetUniverseInfo();
		void ConnectToGalaxyList(const godot::String& ip);
		void DisconnectFromGalaxyList();
		void QueryGalaxyList(const godot::Dictionary& query);
		void PingGalaxy(const godot::String& ip);

		// ####### Galaxy #######

		godot::Ref<UniverseSimulation> StartLocalGalaxy(const godot::String& galaxy_path);
		godot::Ref<UniverseSimulation> StartLocalFragment(const godot::String& fragment_path, const godot::String& fragment_type);
		godot::Ref<UniverseSimulation> StartRemoteGalaxy(const godot::String& galaxy_path);

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		void SimulationStateChanged(const godot::Ref<UniverseSimulation>& simulation, uint64_t load_state);

	private:
		godot::Dictionary m_universe_info_cache;
	};

	struct Universe::Signals
	{
		Signals();

		godot::StringName connected_to_galaxy_list;
		godot::StringName disconnected_from_galaxy_list;
		godot::StringName galaxy_list_query_response;
		godot::StringName galaxy_ping_response;
		godot::StringName simulation_load_state_changed;
	};
}