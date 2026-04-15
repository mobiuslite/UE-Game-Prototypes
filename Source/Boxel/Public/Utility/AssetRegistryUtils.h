// 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssetRegistryUtils.generated.h"

class UResourceDataAsset;
struct FGameplayTag;
/**
 * 
 */
UCLASS()
class BOXEL_API UAssetRegistryUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static TMap<FGameplayTag, const UResourceDataAsset*> GetResourceCDOs();
	
protected:
	
	static TMap<FGameplayTag, const UResourceDataAsset*> AvailableResourceCDOs;
};
