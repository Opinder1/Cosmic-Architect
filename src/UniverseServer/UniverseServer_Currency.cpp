#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetCurrencyInfo(const ID& currency_id)
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

	godot::Dictionary UniverseServer::GetBankInfo(const ID& bank_id)
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

	godot::Dictionary UniverseServer::GetBankInterfaceInfo(const ID& bank_interface_id)
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

	godot::Dictionary UniverseServer::GetGoodInfo(const ID& good_id)
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

	ID UniverseServer::GetUniversalCurrency()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.galaxy_info.find_key("universal_currency");
	}

	ID UniverseServer::GetBankOfInterface(const ID& bank_interface_id)
	{
		return GetBankInterfaceInfo(bank_interface_id).find_key("bank");
	}

	IDVector UniverseServer::GetOwnedCurrencies()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("owned_currencies");
	}

	double UniverseServer::GetBalance(const ID& currency_id)
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

	void UniverseServer::Withdraw(const ID& currency_id, real_t amount, const ID& bank_interface_id)
	{
		if (DeferCommand<&UniverseServer::Withdraw>(currency_id, amount, bank_interface_id))
		{
			return;
		}
	}

	void UniverseServer::Deposit(const ID& currency_id, real_t amount, const ID& bank_interface_id)
	{
		if (DeferCommand<&UniverseServer::Deposit>(currency_id, amount, bank_interface_id))
		{
			return;
		}
	}

	void UniverseServer::Convert(const ID& from_currency_id, const ID& to_currency_id, real_t amount, const ID& bank_interface_id)
	{
		if (DeferCommand<&UniverseServer::Convert>(from_currency_id, to_currency_id, amount, bank_interface_id))
		{
			return;
		}
	}

	void UniverseServer::PayEntity(const ID& currency_id, const ID& entity_id, real_t amount, const ID& bank_interface_id)
	{
		if (DeferCommand<&UniverseServer::PayEntity>(currency_id, entity_id, amount, bank_interface_id))
		{
			return;
		}
	}

	void UniverseServer::BuyGoodWithCurrency(const ID& good_id, const ID& currency_id)
	{
		if (DeferCommand<&UniverseServer::BuyGoodWithCurrency>(good_id, currency_id))
		{
			return;
		}
	}
}