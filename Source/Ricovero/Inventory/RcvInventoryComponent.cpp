#include "Inventory/RcvInventoryComponent.h"

void URcvInventoryComponent::AddKey(FName KeyId)
{
	if (!KeyId.IsNone())
	{
		Keys.Add(KeyId);
		OnInventoryChanged.Broadcast();
	}
}

void URcvInventoryComponent::AddBatteries(int32 Count)
{
	Batteries = FMath::Clamp(Batteries + Count, 0, MaxBatteries);
	OnInventoryChanged.Broadcast();
}

bool URcvInventoryComponent::ConsumeBattery()
{
	if (Batteries <= 0)
	{
		return false;
	}
	--Batteries;
	OnInventoryChanged.Broadcast();
	return true;
}

void URcvInventoryComponent::AddDocument(const FRcvDocument& Document)
{
	if (!Documents.ContainsByPredicate([&Document](const FRcvDocument& D) { return D.Id == Document.Id; }))
	{
		Documents.Add(Document);
		OnInventoryChanged.Broadcast();
	}
}
