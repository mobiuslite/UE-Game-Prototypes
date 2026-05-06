// 

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DeathBringerItemDataAsset.generated.h"


class AStoreItemWorldActor;
class AInventoryItem;

namespace EDeathBringerTeam
{
	enum Type : uint8;
}

UCLASS(BlueprintType, Blueprintable)
class BOXEL_API UDeathBringerItemDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemDescription;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> ItemIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AInventoryItem> ItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AStoreItemWorldActor> WorldActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EDeathBringerTeam::Type> RequiredTeam;
	
	//Makes sure the item isn't potentially shown when purchased
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bConcealOnPurchase;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Cost = 1;
};
