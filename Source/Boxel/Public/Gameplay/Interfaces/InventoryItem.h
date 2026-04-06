// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItem.generated.h"


class UInventoryComponent;

UINTERFACE()
class UInventoryItem : public UInterface
{
	GENERATED_BODY()
};

class BOXEL_API IInventoryItem
{
	GENERATED_BODY()

public:
	
	//When item is put in hands
	virtual void OnEquip(AController* HolderController) {}
	virtual void OnUnequip(AController* HolderController) {}
	
	virtual void OnAddedToInventory(const UInventoryComponent* Inventory, AController* HolderController) {}
	virtual void OnRemovedFromInventory(const UInventoryComponent* Inventory, AController* HolderController) {}
};
