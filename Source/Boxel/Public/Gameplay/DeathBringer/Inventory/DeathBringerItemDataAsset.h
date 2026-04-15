// 

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DeathBringerItemDataAsset.generated.h"


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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MustImplement = "/Script/Boxel.InventoryItem"))
	TSubclassOf<AActor> ItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<EDeathBringerTeam::Type> RequiredTeam;
	
	//Makes sure the item isn't potentially shown when purchased
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bConcealOnPurchase;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Cost = 1;
};
