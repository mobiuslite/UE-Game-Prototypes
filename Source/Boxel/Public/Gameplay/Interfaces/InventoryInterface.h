// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryInterface.generated.h"

class UInventoryComponent;
// This class does not need to be modified.
UINTERFACE()
class UInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BOXEL_API IInventoryInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UInventoryComponent* GetInventory() const = 0;
};
