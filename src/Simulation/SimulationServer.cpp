#include "SimulationServer.h"

#include "Simulation.h"
#include "System.h"
#include "Time.h"

#include "Systems/TickSystem.h"

#include "Util/Debug.h"

namespace sim
{
	std::unique_ptr<SimulationServer> SimulationServer::s_singleton;

	void SimulationServer::Initialize()
	{
		if (s_singleton != nullptr)
		{
			DEBUG_PRINT_ERROR("Already initialised the message registry");
			return;
		}

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
		m_deleter_thread = std::thread(&SimulationServer::DeleteThreadFunc, this);
	}

	SimulationServer::~SimulationServer()
	{
		// Set the stopped flag to true and add all the simulations to the delete queue
		{
			std::unique_lock lock(m_mutex);

			m_stopped = true;

			for (auto&& [id, simulation] : m_simulations)
			{
				if (simulation->IsRunning()) // If we are running and haven't already started to stop
				{
					simulation->Stop();
				}

				// Move the simulation into the delete queue
				simulation.swap(m_delete_queue.emplace_back());
			}

			// Should be left with empty unique ptrs that we can clear
			// No new simulations should be added
			m_simulations.clear();
		}

		// Wait until the deleter deletes all the simulations left
		m_deleter_thread.join();
	}

	UUID SimulationServer::CreateSimulation(double ticks_per_second, bool add_standard_systems)
	{
		UUID id = UUID::GenerateRandom();

		std::unique_lock lock(m_mutex);

		if (m_stopped)
		{
			DEBUG_PRINT_ERROR("The application is stopped so no more simulations can be made");
			return UUID();
		}

		auto&& [it, success] = m_simulations.emplace(id, std::make_unique<Simulation>(id, ticks_per_second));

		if (!success)
		{
			DEBUG_PRINT_ERROR("Failed to create a new simulation");
			return UUID();
		}

		if (add_standard_systems)
		{
			it->second->AddSystem(std::make_unique<TickSystem>(*it->second));
		};

		return id;
	}

	void SimulationServer::AddSystem(UUID id, const SystemEmitter& emitter)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return;
		}

		if (it->second->IsRunning())
		{
			DEBUG_PRINT_ERROR("Can't add a system to the simulation while its running");
			return;
		}

		it->second->AddSystem(emitter);
	}

	void SimulationServer::StartSimulation(UUID id)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return;
		}

		if (it->second->IsRunning())
		{
			DEBUG_PRINT_ERROR("Can't start the simulation while its running");
			return;
		}

		it->second->Start();
	}

	void SimulationServer::StopSimulation(UUID id)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return;
		}

		if (!it->second->IsRunning())
		{
			DEBUG_PRINT_ERROR("Can't stop the simulation if its not running");
			return;
		}

		it->second->Stop();
	}

	void SimulationServer::DeleteSimulation(UUID id)
	{
		std::unique_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return;
		}

		if (it->second->IsRunning())
		{
			DEBUG_PRINT_ERROR("Can't delete the simulation if its running");
			return;
		}

		// Move the simulation into the delete queue
		it->second.swap(m_delete_queue.emplace_back());

		m_simulations.erase(it);
	}

	void SimulationServer::StopAndDeleteSimulation(UUID id)
	{
		std::unique_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return;
		}

		// Stop the simulation if its running
		if (it->second->IsRunning())
		{
			it->second->Stop();
		}

		// Move the simulation into the delete queue
		it->second.swap(m_delete_queue.emplace_back());

		m_simulations.erase(it);
	}

	void SimulationServer::SendMessage(UUID id, const MessagePtr& message)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return;
		}

		if (!it->second->IsRunning())
		{
			DEBUG_PRINT_ERROR("Can't send a message if the simulation is not running");
			return;
		}

		it->second->PostMessageFromUnattested(message);
	}

	void SimulationServer::SendMessages(UUID id, const MessageQueue& messages)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return;
		}

		if (!it->second->IsRunning())
		{
			DEBUG_PRINT_ERROR("Can't send a message if the simulation is not running");
			return;
		}

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

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return false;
		}

		return it->second->IsRunning();
	}

	bool SimulationServer::IsSimulationStopping(UUID id)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return false;
		}

		return it->second->IsStopping();
	}

	bool SimulationServer::ApplyToSimulation(UUID id, const SimulationApplicator& callback)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return false;
		}

		callback(*it->second);

		return true;
	}

	void SimulationServer::DeleteThreadFunc()
	{
		std::unique_lock lock(m_mutex);

		// Keep looping as long as we haven't stopped and the delete queue is not empty
		while (!(m_stopped && m_delete_queue.empty()))
		{
			for (auto it = m_delete_queue.begin(); it != m_delete_queue.end(); it++)
			{
				if (!it->get()->IsRunning()) // If we are not running then we have stopped
				{
					it = m_delete_queue.erase(it); // Erase this simulation finally
				}
			}

			// Every second we check if the simulations have stopped
			// This should be plenty of time for the simulations to stop and makes us lock very rarely
			lock.unlock();
			std::this_thread::sleep_for(1s);
			lock.lock();
		}

		DEBUG_ASSERT(m_delete_queue.empty(), "Delete queue should be empty");
		DEBUG_ASSERT(m_simulations.empty(), "Simulations should be empty");
	}
}