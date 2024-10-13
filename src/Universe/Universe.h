#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/rid.hpp>

#include <robin_hood/robin_hood.h>

#include <optional>

namespace voxel_game
{
	class UniverseSimulation;
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

	public:
		static void _bind_methods();
		static void _cleanup_methods();

	private:
		godot::Dictionary m_universe_info_cache;
	};

	struct Universe::SignalStrings
	{
		SignalStrings();

		godot::StringName connected_to_galaxy_list;
		godot::StringName disconnected_from_galaxy_list;
		godot::StringName galaxy_list_query_response;
		godot::StringName galaxy_ping_response;
	};
}