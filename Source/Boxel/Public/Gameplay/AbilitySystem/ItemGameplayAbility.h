// 

#pragma once

#include "CoreMinimal.h"
#include "MobiusAbilitySystem/MAGameplayAbility.h"
#include "ItemGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class BOXEL_API UItemGameplayAbility : public UMAGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable)
	void RemoveOwningItemFromInventory(const bool bDestroy = true);
};
