#pragma once

#include "FlecsWorld.h"

#include <robin_hood/robin_hood.h>

namespace voxel_world
{
	class UniverseSimulation : public FlecsWorld
	{
		GDCLASS(UniverseSimulation, FlecsWorld);

	private:
		struct DeferredCommand
		{
			size_t id;
			godot::Array args;
		};

	public:
		UniverseSimulation();
		~UniverseSimulation();

		// Start a galaxy locally
		void StartLocalGalaxy(const godot::String& galaxy_path);

		// Start a fragment of a galaxy network locally
		void StartLocalFragment(const godot::String& fragment_path);

		// Attempt to connect to a galaxy network hosted remotely
		void StartRemoteGalaxy(const godot::String& galaxy_path);

		// Stop the galaxy running
		void StopGalaxy();

		// When connected to a remote galaxy we may need to login
		void CreateAccount();
		void LoginAccount();
		void LogoutAccount();

		// Send a galaxy command to the server
		bool SendGalaxyCommand(const godot::String& command, const godot::Array& args);
		bool SendGalaxyCommandInternal(size_t command_id, const godot::Array& args);

	protected:
		static void _bind_methods();

	private:
		godot::String m_path;

		robin_hood::unordered_map<godot::String, size_t> m_command_names;

		bool m_deferred;
		std::vector<DeferredCommand> m_command_queue;
		flecs::entity_t m_command_event;
	};
}