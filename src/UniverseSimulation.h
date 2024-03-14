#pragma once

#include "FlecsWorld.h"

#include <robin_hood/robin_hood.h>

namespace voxel_world
{
	class UniverseSimulation : public FlecsWorld
	{
		GDCLASS(UniverseSimulation, FlecsWorld);

	public:
		UniverseSimulation();
		~UniverseSimulation();

		// Start a galaxy locally
		void StartLocalGalaxy(const godot::String& galaxy_path);

		// Start a fragment of a galaxy network locally
		void StartLocalFragment(const godot::String& fragment_path, const godot::String& fragment_type);

		// Attempt to connect to a galaxy network hosted remotely
		void StartRemoteGalaxy(const godot::String& galaxy_path);

		// Stop the galaxy running
		void StopGalaxy();

		// When connected to a remote galaxy we may need to login
		void CreateAccount(const godot::String& username, const godot::String& password_hash);
		void LoginAccount(const godot::String& username, const godot::String& password_hash);
		void LogoutAccount();

		uint64_t CreateInstance(godot::RID mesh);

	protected:
		static void _bind_methods();

	private:
		godot::String m_path;
		godot::String m_fragment_type;

		godot::Signal m_remote_on_connect;
		godot::Signal m_remote_on_disconnect;
	};
}