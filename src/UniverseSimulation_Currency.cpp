#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetCurrencyInfo(UUID currency_id)
	{
		std::shared_lock lock(m_cache_mutex);

		auto it = m_info_cache.currency_info_map.find(currency_id);

		if (it != m_info_cache.currency_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	godot::Dictionary UniverseSimulation::GetBankInfo(UUID bank_id)
	{
		std::shared_lock lock(m_cache_mutex);

		auto it = m_info_cache.bank_info_map.find(bank_id);

		if (it != m_info_cache.bank_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	godot::Dictionary UniverseSimulation::GetBankInterfaceInfo(UUID bank_interface_id)
	{
		std::shared_lock lock(m_cache_mutex);

		auto it = m_info_cache.bank_interface_info_map.find(bank_interface_id);

		if (it != m_info_cache.bank_interface_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	godot::Dictionary UniverseSimulation::GetGoodInfo(UUID good_id)
	{
		std::shared_lock lock(m_cache_mutex);

		auto it = m_info_cache.good_info_map.find(good_id);

		if (it != m_info_cache.good_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	UUID UniverseSimulation::GetUniversalCurrency()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_info_cache.galaxy_info.find_key("universal_currency");
	}

	UUID UniverseSimulation::GetBankOfInterface(UUID bank_interface_id)
	{
		return GetBankInterfaceInfo(bank_interface_id).find_key("bank");
	}

	UUIDVector UniverseSimulation::GetOwnedCurrencies()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_info_cache.player_info.find_key("owned_currencies");
	}

	double UniverseSimulation::GetBalance(UUID currency_id)
	{
		std::shared_lock lock(m_cache_mutex);
		
		godot::Dictionary balances = m_info_cache.player_info.find_key("balances");

		if (balances.is_empty())
		{
			return NAN;
		}

		return balances.find_key(currency_id);
	}

	void UniverseSimulation::Withdraw(UUID currency_id, real_t amount, UUID bank_interface_id)
	{
		SIM_DEFER_COMMAND(k_commands->withdraw, currency_id, amount, bank_interface_id);
	}

	void UniverseSimulation::Deposit(UUID currency_id, real_t amount, UUID bank_interface_id)
	{
		SIM_DEFER_COMMAND(k_commands->deposit, currency_id, amount, bank_interface_id);
	}

	void UniverseSimulation::Convert(UUID from_currency_id, UUID to_currency_id, real_t amount, UUID bank_interface_id)
	{
		SIM_DEFER_COMMAND(k_commands->convert, from_currency_id, to_currency_id, amount, bank_interface_id);
	}

	void UniverseSimulation::PayEntity(UUID currency_id, UUID entity_id, real_t amount, UUID bank_interface_id)
	{
		SIM_DEFER_COMMAND(k_commands->pay_entity, currency_id, entity_id, amount, bank_interface_id);
	}

	void UniverseSimulation::BuyGoodWithCurrency(UUID good_id, UUID currency_id)
	{
		SIM_DEFER_COMMAND(k_commands->buy_good_with_currency, good_id, currency_id);
	}
}