#include "UniverseSimulation.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetCurrencyInfo(UUID currency_id)
	{
		return {};
	}

	godot::Dictionary UniverseSimulation::GetBankInfo(UUID bank_id)
	{
		return {};
	}

	godot::Dictionary UniverseSimulation::GetBankInterfaceInfo(UUID bank_interface_id)
	{
		return {};
	}

	godot::Dictionary UniverseSimulation::GetGoodInfo(UUID good_id)
	{
		return {};
	}

	UniverseSimulation::UUID UniverseSimulation::GetBankOfInterface(UUID bank_interface_id)
	{
		return {};
	}

	UniverseSimulation::UUIDVector UniverseSimulation::GetOwnedCurrencies()
	{
		return {};
	}

	double UniverseSimulation::GetBalance(UUID currency_id)
	{
		return 0.0;
	}

	void UniverseSimulation::Withdraw(UUID currency_id, double amount, UUID bank_interface_id)
	{

	}

	void UniverseSimulation::Deposit(UUID currency_id, double amount, UUID bank_interface_id)
	{

	}

	void UniverseSimulation::Convert(UUID from_currency_id, UUID to_currency_id, double amount, UUID bank_interface_id)
	{

	}

	void UniverseSimulation::PayEntity(UUID currency_id, UUID entity_id, double amount, UUID bank_interface_id)
	{

	}

	void UniverseSimulation::BuyGoodWithCurrency(UUID good_id, UUID currency_id)
	{

	}
}