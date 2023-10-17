#include "SimulationServer.h"

#include "Simulation.h"
#include "System.h"
#include "Time.h"

#include <godot_cpp/core/error_macros.hpp>

namespace sim
{
	std::unique_ptr<SimulationServer> SimulationServer::s_singleton;

	void SimulationServer::Initialize()
	{
		CRASH_COND(s_singleton != nullptr);

		s_singleton = std::make_unique<SimulationServer>();
	}

	void SimulationServer::Uninitialize()
	{
		s_singleton.reset();
	}

	SimulationServer* SimulationServer::GetSingleton()
	{
		return s_singleton.get();
	}

	SimulationServer::SimulationServer() :
		m_stopped(false)
	{

	}

	SimulationServer::~SimulationServer()
	{
		{
			std::unique_lock lock(m_mutex);
			m_stopped = true;
		}

		for (auto&& [id, simulation] : m_simulations)
		{
			if (simulation->IsRunning()) // If we are running and haven't already started to stop
			{
				simulation->Stop();
			}
		}

		// Wait for all simulations to stop
		for (auto&& [id, simulation] : m_simulations)
		{
			while (simulation->IsRunning())
			{
				std::this_thread::yield();
			}
		}

		// When we delete the simulations each one will try to join its thread
	}

	UUID SimulationServer::CreateSimulation(double ticks_per_second)
	{
		UUID id = UUID::GenerateRandom();

		std::unique_lock lock(m_mutex);

		ERR_FAIL_COND_V_MSG(m_stopped, UUID(), "The application is stopped so no more simulations can be made");

		auto&& [it, success] = m_simulations.emplace(id, std::make_unique<Simulation>(id, ticks_per_second));

		ERR_FAIL_COND_V_MSG(!success, UUID(), "Failed to create a new simulation");

		return id;
	}

	void SimulationServer::AddSystem(UUID id, const SystemEmitter& emitter)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		ERR_FAIL_COND_MSG(it == m_simulations.end(), "No simulation exists with the id " + id.ToGodotString());
		ERR_FAIL_COND_MSG(it->second->IsRunning(), "Can't add a system to the simulation while its running");

		it->second->AddSystem(emitter);
	}

	void SimulationServer::StartSimulation(UUID id)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		ERR_FAIL_COND_MSG(it == m_simulations.end(), "No simulation exists with the id " + id.ToGodotString());
		ERR_FAIL_COND_MSG(it->second->IsRunning(), "Can't start the simulation while its running");

		it->second->Start();
	}

	void SimulationServer::StopSimulation(UUID id)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		ERR_FAIL_COND_MSG(it == m_simulations.end(), "No simulation exists with the id " + id.ToGodotString());
		ERR_FAIL_COND_MSG(!it->second->IsRunning(), "Can't stop the simulation if its not running");

		it->second->Stop();
	}

	void SimulationServer::DeleteSimulation(UUID id)
	{
		std::unique_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		ERR_FAIL_COND_MSG(it == m_simulations.end(), "No simulation exists with the id " + id.ToGodotString());
		ERR_FAIL_COND_MSG(it->second->IsRunning(), "Can't delete the simulation if its running");

		m_simulations.erase(it);
	}

	void SimulationServer::StopAndDeleteSimulation(UUID id)
	{
		std::unique_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		ERR_FAIL_COND_MSG(it == m_simulations.end(), "No simulation exists with the id " + id.ToGodotString());

		Simulation* sim = it->second.release(); // Release before erasing

		m_simulations.erase(it);

		sim->StopAndDelete(); // Call on the released which should delete itself
	}

	void SimulationServer::SendMessage(UUID id, const MessagePtr& message)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		ERR_FAIL_COND_MSG(it == m_simulations.end(), "No simulation exists with the id " + id.ToGodotString());
		ERR_FAIL_COND_MSG(!it->second->IsRunning(), "Can't send a message if the simulation is not running");

		it->second->PostMessageFromUnattested(message);
	}

	void SimulationServer::SendMessages(UUID id, const MessageQueue& messages)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		ERR_FAIL_COND_MSG(it == m_simulations.end(), "No simulation exists with the id " + id.ToGodotString());
		ERR_FAIL_COND_MSG(!it->second->IsRunning(), "Can't send a message if the simulation is not running");

		it->second->PostMessagesFromUnattested(messages);
	}

	bool SimulationServer::IsSimulation(UUID id)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		return it != m_simulations.end();
	}

	bool SimulationServer::IsSimulationRunning(UUID id)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		ERR_FAIL_COND_V_MSG(it == m_simulations.end(), false, "No simulation exists with the id " + id.ToGodotString());

		return it->second->IsRunning();
	}

	bool SimulationServer::IsSimulationStopping(UUID id)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		ERR_FAIL_COND_V_MSG(it == m_simulations.end(), false, "No simulation exists with the id " + id.ToGodotString());

		return it->second->IsStopping();
	}

	bool SimulationServer::ApplyToSimulation(UUID id, const SimulationApplicator& callback)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		ERR_FAIL_COND_V_MSG(it == m_simulations.end(), false, "No simulation exists with the id " + id.ToGodotString());

		callback(*it->second);

		return true;
	}
}