// 

#pragma once

#include "CoreMinimal.h"
#include "Core/DeathBringer/DeathBringerGameMode.h"
#include "Engine/DataAsset.h"
#include "StoreItemSetDataAsset.generated.h"

class UDeathBringerItemDataAsset;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class BOXEL_API UStoreItemSetDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UDeathBringerItemDataAsset*> ItemSet;
};
