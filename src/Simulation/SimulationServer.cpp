#include "SimulationServer.h"
#include "Events.h"

#include "Simulation.h"
#include "Time.h"

#include "Systems/NewEntitiesSystem.h"
#include "Systems/DeletedEntitiesSystem.h"
#include "Systems/LinkedMessagerSystem.h"
#include "Systems/LinkedSimulationSystem.h"
#include "Systems/LinkedRemoteSimulationSystem.h"

#include "Network/NetworkPeerSystem.h"
#include "Network/NetworkServerSystem.h"

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
		std::unique_lock lock(m_mutex);

		m_stopped = true;

		for (auto&& [id, simulation] : m_simulations)
		{
			if (simulation->messager.IsRunning()) // If we are running and haven't already started to stop
			{
				simulation->messager.Stop();

				// If we are manually ticked then give ownership to the deleter so it can keep ticking
				// This is because that thread will no longer be able to tick it
				if (simulation->messager.IsManuallyTicked())
				{
					DEBUG_PRINT_ERROR("Manually ticked simulations should be fully stopped before they are deleted");
					simulation->messager.ThreadTransferObject(m_deleter_thread.get_id());
				}
			}

			// Move the simulation into the delete queue
			simulation.swap(m_delete_queue.emplace_back());
		}

		// Should be left with empty unique ptrs that we can clear
		// No new simulations should be added
		m_simulations.clear();

		lock.unlock();

		// Wait until the deleter deletes all the simulations left
		m_deleter_thread.join();
	}

	void SimulationServer::StartNetworking()
	{
		m_network_simulation = SimulationServer::GetSingleton()->CreateSimulation(60, true);

		SimulationServer::GetSingleton()->AddSystem<NetworkServerSystem>(m_network_simulation);
		SimulationServer::GetSingleton()->AddSystem<NetworkPeerSystem>(m_network_simulation);

		SimulationServer::GetSingleton()->StartSimulation(m_network_simulation);
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

		auto&& [it, success] = m_simulations.emplace(id, std::make_unique<Simulation>(*this, id, ticks_per_second));

		if (!success)
		{
			DEBUG_PRINT_ERROR("Failed to create a new simulation");
			return UUID();
		}

		if (add_standard_systems)
		{
			Simulation& simulation = *it->second;

			auto AddSystem = [&simulation](const SimulationApplicator& initialize, const SimulationApplicator& shutdown)
			{
				initialize(simulation);
				simulation.system_shutdowns.push_back(shutdown);
			};

			AddSystem(&NewEntitiesSystem::OnInitialize, &NewEntitiesSystem::OnShutdown);
			AddSystem(&DeletedEntitiesSystem::OnInitialize, &DeletedEntitiesSystem::OnShutdown);
			AddSystem(&LinkedMessagerSystem::OnInitialize, &LinkedMessagerSystem::OnShutdown);
			AddSystem(&LinkedSimulationSystem::OnInitialize, &LinkedSimulationSystem::OnShutdown);
			AddSystem(&LinkedRemoteSimulationSystem::OnInitialize, &LinkedRemoteSimulationSystem::OnShutdown);
		};

		return id;
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

		Simulation& simulation = *it->second;

		// Stop the simulation if its running
		if (simulation.messager.IsRunning())
		{
			// Call stop first in case we transfer thread ownership
			simulation.messager.Stop();

			// If we are manually ticked then give ownership to the deleter so it can keep ticking
			// This is because that thread will no longer be able to tick it
			if (simulation.messager.IsManuallyTicked())
			{
				if (!simulation.messager.ThreadOwnsObject())
				{
					DEBUG_PRINT_ERROR("Manually ticked simulations should be stopped and deleted by the managing thread");
					return;
				}

				simulation.messager.ThreadTransferObject(m_deleter_thread.get_id());
			}
		}

		// Move the simulation into the delete queue
		it->second.swap(m_delete_queue.emplace_back());

		m_simulations.erase(it);
	}

	void SimulationServer::AddSystem(UUID id, const SimulationApplicator& initialize, const SimulationApplicator& shutdown)
	{
		std::unique_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return;
		}

		Simulation& simulation = *it->second;

		DEBUG_ASSERT(!simulation.messager.ObjectOwned(), "This simulation should not be owned by a thread when adding a system");

		if (simulation.messager.IsRunning())
		{
			DEBUG_PRINT_ERROR("This simulation should not be running when trying to add a system");
			return;
		}

		initialize(simulation);

		simulation.system_shutdowns.push_back(shutdown);
	}

	bool SimulationServer::StartSimulation(UUID id)
	{
		bool success = false;

		ApplyToSimulation(id, [&success](SimulationMessager& messager)
		{
			success = messager.Start(false);
		});

		return success;
	}

	SimulationMessager* SimulationServer::StartManualSimulation(UUID id)
	{
		SimulationMessager* simulation_ptr = nullptr;

		ApplyToSimulation(id, [&simulation_ptr](SimulationMessager& messager)
		{
			if (messager.Start(true))
			{
				simulation_ptr = &messager;
			}
		});

		return simulation_ptr;
	}

	void SimulationServer::StopSimulation(UUID id)
	{
		ApplyToSimulation(id, [](SimulationMessager& messager)
		{
			messager.Stop();
		});
	}

	void SimulationServer::SendMessage(UUID id, const MessagePtr& message)
	{
		ApplyToSimulation(id, [&message](SimulationMessager& messager)
		{
			if (!messager.IsRunning())
			{
				DEBUG_PRINT_ERROR("Can't send a message if the simulation is not running");
				return;
			}

			messager.PostMessageFromUnattested(message);
		});
	}

	void SimulationServer::SendMessages(UUID id, const MessageQueue& messages)
	{
		ApplyToSimulation(id, [&messages](SimulationMessager& messager)
		{
			if (!messager.IsRunning())
			{
				DEBUG_PRINT_ERROR("Can't send a message if the simulation is not running");
				return;
			}

			messager.PostMessagesFromUnattested(messages);
		});
	}

	bool SimulationServer::IsSimulation(UUID id)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		return it != m_simulations.end();
	}

	SimulationServer::Result SimulationServer::IsSimulationRunning(UUID id)
	{
		Result result = Result::Invalid;

		ApplyToSimulation(id, [&result](SimulationMessager& messager)
		{
			result = messager.IsRunning() ? Result::True : Result::False;
		});

		return result;
	}

	SimulationServer::Result SimulationServer::IsSimulationManuallyTicked(UUID id)
	{
		Result result = Result::Invalid;

		ApplyToSimulation(id, [&result](SimulationMessager& messager)
		{
			result = messager.IsManuallyTicked() ? Result::True : Result::False;
		});

		return result;
	}

	SimulationServer::Result SimulationServer::IsSimulationStopping(UUID id)
	{
		Result result = Result::Invalid;

		ApplyToSimulation(id, [&result](SimulationMessager& messager)
		{
			result = messager.IsStopping() ? Result::True : Result::False;
		});

		return result;
	}

	bool SimulationServer::ApplyToSimulation(UUID id, const MessagerApplicator& callback)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
			return false;
		}

		callback(it->second->messager);

		return true;
	}

	void SimulationServer::AttemptFreeMemory()
	{
		MessagePtr event = std::make_shared<AttemptFreeMemoryMessage>();

		for (auto&& [id, simulation] : m_simulations)
		{
			simulation->messager.PostMessageFromUnattested(event);
		}

		std::unique_lock lock(m_mutex);

		m_simulations.compact();
		m_delete_queue.shrink_to_fit();
	}

	void SimulationServer::DeleteThreadFunc()
	{
		std::unique_lock lock(m_mutex);

		// Keep looping as long as we haven't stopped and the delete queue is not empty
		while (!(m_stopped && m_delete_queue.empty()))
		{
			// Every second we check if the simulations have stopped
			// This should be plenty of time for the simulations to stop and makes us lock very rarely
			lock.unlock();
			std::this_thread::sleep_for(1s);
			lock.lock();

			for (auto it = m_delete_queue.begin(); it != m_delete_queue.end();)
			{
				Simulation& simulation = **it;

				if (!simulation.messager.IsRunning())
				{
					// If its not running then we can erase the simulation safely
					it = m_delete_queue.erase(it);
					continue;
				}

				// If we are manually ticked then make sure to keep ticking so that we handle unlinks
				if (simulation.messager.IsManuallyTicked())
				{
					lock.unlock();
					simulation.messager.ManualTick();
					lock.lock();
					break; // Do one manual tick as in the time it took the delete queue could have reallocated
				}

				it++;
			}
		}

		DEBUG_ASSERT(m_delete_queue.empty(), "Delete queue should be empty");
		DEBUG_ASSERT(m_simulations.empty(), "Simulations should be empty");
	}
}