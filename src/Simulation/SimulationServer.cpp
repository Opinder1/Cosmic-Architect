#include "SimulationServer.h"
#include "Simulation.h"
#include "SimulationBuilder.h"
#include "Events.h"

#include "Network/NetworkPeerSystem.h"
#include "Network/NetworkServerSystem.h"

#include "Util/Time.h"
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
		DEBUG_ASSERT(s_singleton.get() != nullptr, "THe singleton should be valid");
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
		m_network_simulation = CreateSimulation(std::make_unique<EmptySimulationBuilder>(), sim::SimulationServer::CreateMethod::Thread);

		//AddSystem<NetworkServerSystem>(m_network_simulation);
		//AddSystem<NetworkPeerSystem>(m_network_simulation);

		StartSimulation(m_network_simulation);
	}

	std::vector<UUID> SimulationServer::GetAllSimulations()
	{
		std::shared_lock lock(m_mutex);

		std::vector<UUID> out;

		out.reserve(m_simulations.size());

		for (auto&& [id, simulation] : m_simulations)
		{
			out.push_back(id);
		}

		return out;
	}

	UUID SimulationServer::CreateSimulation(std::unique_ptr<SimulationBuilder>&& builder, CreateMethod method)
	{
		UUID id = UUID::GenerateRandom();

		SimulationPtr simulation = std::make_unique<Simulation>(*this, id);

		if (method == CreateMethod::Local) // Build before locking if we build local so we don't keep the server waiting when building
		{
			builder->Build(*simulation);
		}

		std::unique_lock lock(m_mutex);

		if (m_stopped)
		{
			DEBUG_PRINT_ERROR("The application is stopped so no more simulations can be made");
			return UUID();
		}

		auto&& [it, success] = m_simulations.emplace(id, std::move(simulation));

		if (!success)
		{
			DEBUG_PRINT_ERROR("Failed to insert a new simulation");
			return UUID();
		}

		if (method == CreateMethod::Thread)
		{
			Simulation& simulation = *it->second;

			simulation.messager.BuildStart(std::move(builder), simulation);
		}

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
		}

		// Move the simulation into the delete queue
		it->second.swap(m_delete_queue.emplace_back());

		m_simulations.erase(it);
	}

	bool SimulationServer::StartSimulation(UUID id)
	{
		bool success = false;

		ApplyToSimulation(id, [&success](SimulationMessager& messager)
		{
			success = messager.Start();
		});

		return success;
	}

	void SimulationServer::StopSimulation(UUID id)
	{
		ApplyToSimulation(id, [](SimulationMessager& messager)
		{
			messager.Stop();
		});
	}

	bool SimulationServer::ThreadAcquireSimulation(UUID id)
	{
		bool success = false;

		ApplyToSimulation(id, [&success](SimulationMessager& messager)
		{
			success = messager.ThreadAcquire();
		});

		return success;
	}

	SimulationMessager* SimulationServer::TryGetAcquiredSimulation(UUID id)
	{
		SimulationMessager* simulation_ptr = nullptr;

		ApplyToSimulation(id, [&simulation_ptr](SimulationMessager& messager)
		{
			if (messager.IsExternallyTicked())
			{
				simulation_ptr = &messager;
			}
		});

		return simulation_ptr;
	}

	bool SimulationServer::ThreadReleaseSimulation(UUID id)
	{
		bool success = false;

		ApplyToSimulation(id, [&success](SimulationMessager& messager)
		{
			success = messager.ThreadRelease();
		});

		return success;
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

	SimulationServer::Result SimulationServer::IsSimulationExternallyTicked(UUID id)
	{
		Result result = Result::Invalid;

		ApplyToSimulation(id, [&result](SimulationMessager& messager)
		{
			result = messager.IsExternallyTicked() ? Result::True : Result::False;
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

	SimulationServer::Result SimulationServer::ThreadOwnsSimulation(UUID id)
	{
		Result result = Result::Invalid;

		ApplyToSimulation(id, [&result](SimulationMessager& messager)
		{
			result = messager.ThreadOwnsSimulation() ? Result::True : Result::False;
		});

		return result;
	}

	void SimulationServer::ApplyToSimulation(UUID id, const MessagerApplicator& callback)
	{
		std::shared_lock lock(m_mutex);

		auto it = m_simulations.find(id);

		if (it == m_simulations.end())
		{
			DEBUG_PRINT_ERROR("No simulation exists with the id " + id.ToGodotString());
		}

		callback(it->second->messager);
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
				}
				else
				{
					it++;
				}
			}
		}

		DEBUG_ASSERT(m_delete_queue.empty(), "Delete queue should be empty");
		DEBUG_ASSERT(m_simulations.empty(), "Simulations should be empty");
	}
}