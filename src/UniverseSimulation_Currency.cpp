#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetCurrencyInfo(UUID currency_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(currency_id);
	}

	godot::Dictionary UniverseSimulation::GetBankInfo(UUID bank_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(bank_id);
	}

	godot::Dictionary UniverseSimulation::GetBankInterfaceInfo(UUID bank_interface_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(bank_interface_id);
	}

	godot::Dictionary UniverseSimulation::GetGoodInfo(UUID good_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(good_id);
	}

	UniverseSimulation::UUID UniverseSimulation::GetUniversalCurrency()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.universal_currency;
	}

	UniverseSimulation::UUID UniverseSimulation::GetBankOfInterface(UUID bank_interface_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(bank_interface_id).find_key("bank");
	}

	UniverseSimulation::UUIDVector UniverseSimulation::GetOwnedCurrencies()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.player_info.find_key("owned_currencies");
	}

	double UniverseSimulation::GetBalance(UUID currency_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.player_info.find_key("balance");
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