#include "UniverseSimulation.h"

#include "Util/Debug.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_world
{
	UniverseSimulation::UniverseSimulation()
	{}

	UniverseSimulation::~UniverseSimulation()
	{}

	void UniverseSimulation::StartLocalGalaxy(const godot::String& galaxy_path)
	{
		m_world.reset();
		m_path = galaxy_path;
	}

	void UniverseSimulation::StartLocalFragment(const godot::String& fragment_path)
	{

	}

	void UniverseSimulation::StartRemoteGalaxy(const godot::String& galaxy_path)
	{

	}

	void UniverseSimulation::StopGalaxy()
	{
		m_world.reset();
		m_path = godot::String();
	}

	void UniverseSimulation::CreateAccount()
	{

	}

	void UniverseSimulation::LoginAccount()
	{

	}

	void UniverseSimulation::LogoutAccount()
	{

	}

	bool UniverseSimulation::SendGalaxyCommand(const godot::String& command, const godot::Array& args)
	{
		auto it = m_command_names.find(command);

		if (it == m_command_names.end())
		{
			return;
		}

		return SendGalaxyCommandInternal(it->second, args);
	}

	bool UniverseSimulation::SendGalaxyCommandInternal(size_t command_id, const godot::Array& args)
	{
		auto command = m_world.event(m_command_event);

		command.ctx(&args);

		if (m_world.is_deferred())
		{
			command.enqueue();
		}
		else
		{
			command.emit();
		}
	}

	void UniverseSimulation::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("start_local_galaxy", "galaxy_path"), &UniverseSimulation::StartLocalGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("start_local_fragment", "fragment_path"), &UniverseSimulation::StartLocalFragment);
		godot::ClassDB::bind_method(godot::D_METHOD("start_remote_galaxy", "galaxy_path"), &UniverseSimulation::StartRemoteGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("stop_galaxy"), &UniverseSimulation::StopGalaxy);
		godot::ClassDB::bind_method(godot::D_METHOD("create_account"), &UniverseSimulation::CreateAccount);
		godot::ClassDB::bind_method(godot::D_METHOD("login_account"), &UniverseSimulation::LoginAccount);
		godot::ClassDB::bind_method(godot::D_METHOD("logout_account"), &UniverseSimulation::LogoutAccount);
		godot::ClassDB::bind_method(godot::D_METHOD("send_galaxy_command", "command", "args"), &UniverseSimulation::SendGalaxyCommand);
	}
}