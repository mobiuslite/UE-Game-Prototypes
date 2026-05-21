// 


#include "Gameplay/AbilitySystem/ItemGameplayAbility.h"

#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"
#include "Utility/MobiusUtils.h"

void UItemGameplayAbility::RemoveOwningItemFromInventory(const bool bDestroy)
{
	if (!HasAuthority(&CurrentActivationInfo)) return;
	
	UInventoryComponent* Inventory;
	if (UMobiusUtils::GetInventory(GetOwningActorFromActorInfo(), Inventory))
	{
		AInventoryItem* Item = Cast<AInventoryItem>(GetCurrentSourceObject());
		Inventory->RemoveItem(Item);
		
		if (bDestroy)
		{
			Item->Destroy();
		}
	}
}
