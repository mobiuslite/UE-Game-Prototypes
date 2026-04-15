// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItem.generated.h"


class UInventoryComponent;

UENUM(BlueprintType)
namespace EInventoryItem
{
	enum Type : uint8
	{
		None,
		
		LargeItem,
		SmallItem,
		Utility
	};
}

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
	//These should only be called by local or authority, simulated proxies should not call these
	virtual void OnEquip(AController* HolderController) {}
	virtual void OnUnequip(AController* HolderController) {}
	
	virtual void OnAddedToInventory(const UInventoryComponent* Inventory, AController* HolderController) {}
	virtual void OnRemovedFromInventory(const UInventoryComponent* Inventory, AController* HolderController) {}
	
	virtual bool CanBePickedUp(const APawn* PawnHolder) const {return true;}
	virtual TEnumAsByte<EInventoryItem::Type> GetItemType() const { return EInventoryItem::None; }
};
