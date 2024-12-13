#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetCurrencyInfo(const UUID& currency_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

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

	godot::Dictionary UniverseSimulation::GetBankInfo(const UUID& bank_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

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

	godot::Dictionary UniverseSimulation::GetBankInterfaceInfo(const UUID& bank_interface_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

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

	godot::Dictionary UniverseSimulation::GetGoodInfo(const UUID& good_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

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
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.galaxy_info.find_key("universal_currency");
	}

	UUID UniverseSimulation::GetBankOfInterface(const UUID& bank_interface_id)
	{
		return GetBankInterfaceInfo(bank_interface_id).find_key("bank");
	}

	UUIDVector UniverseSimulation::GetOwnedCurrencies()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("owned_currencies");
	}

	double UniverseSimulation::GetBalance(const UUID& currency_id)
	{
		godot::Dictionary balances;

		{
			std::shared_lock lock(m_info_cache.mutex);
			balances = m_info_cache.player_info.find_key("balances");
		}

		if (balances.is_empty())
		{
			return NAN;
		}

		return balances.find_key(currency_id);
	}

	void UniverseSimulation::Withdraw(const UUID& currency_id, real_t amount, const UUID& bank_interface_id)
	{
		if (DeferCommand<&UniverseSimulation::Withdraw>(currency_id, amount, bank_interface_id))
		{
			return;
		}
	}

	void UniverseSimulation::Deposit(const UUID& currency_id, real_t amount, const UUID& bank_interface_id)
	{
		if (DeferCommand<&UniverseSimulation::Deposit>(currency_id, amount, bank_interface_id))
		{
			return;
		}
	}

	void UniverseSimulation::Convert(const UUID& from_currency_id, const UUID& to_currency_id, real_t amount, const UUID& bank_interface_id)
	{
		if (DeferCommand<&UniverseSimulation::Convert>(from_currency_id, to_currency_id, amount, bank_interface_id))
		{
			return;
		}
	}

	void UniverseSimulation::PayEntity(const UUID& currency_id, const UUID& entity_id, real_t amount, const UUID& bank_interface_id)
	{
		if (DeferCommand<&UniverseSimulation::PayEntity>(currency_id, entity_id, amount, bank_interface_id))
		{
			return;
		}
	}

	void UniverseSimulation::BuyGoodWithCurrency(const UUID& good_id, const UUID& currency_id)
	{
		if (DeferCommand<&UniverseSimulation::BuyGoodWithCurrency>(good_id, currency_id))
		{
			return;
		}
	}
}