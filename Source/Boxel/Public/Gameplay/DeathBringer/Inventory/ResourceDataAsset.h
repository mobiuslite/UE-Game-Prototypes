// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ResourceDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class BOXEL_API UResourceDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag ResourceTag;
	
	//0 = Infinite
	UPROPERTY(EditDefaultsOnly)
	int MaxResourceCount;
};
