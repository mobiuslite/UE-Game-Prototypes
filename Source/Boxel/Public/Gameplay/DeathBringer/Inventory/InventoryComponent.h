// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Interfaces/InventoryItem.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryComponent.generated.h"


class UResourceDataAsset;

USTRUCT()
struct FResourceData : public  FFastArraySerializerItem
{
	GENERATED_BODY()
	
	UPROPERTY()
	FGameplayTag ResourceTag;
	
	UPROPERTY()
	int ResourceCount;
	
	void PreReplicatedRemove(const struct FResources& InArraySerializer) {}
	void PostReplicatedAdd(const struct FResources& InArraySerializer) {}
	void PostReplicatedChange(const struct FResources& InArraySerializer) {}
};

USTRUCT()
struct FResources : public FFastArraySerializer
{
	GENERATED_BODY()
	
	void AddResource(const FGameplayTag& Tag, const int Count);
	bool ConsumeResource(const FGameplayTag& Tag, const int Count);
	void ClearResources();
	
	int GetResourceCount(const FGameplayTag& Tag) const;
	int GetResourcesNum() const { return Resources.Num(); }
	const FResourceData& GetResourceAt(const int Index) const { return Resources[Index]; }
	FResourceData& GetResourceAt(const int Index) { return Resources[Index]; }
	
protected:
	UPROPERTY()
	TArray<FResourceData> Resources;
	
public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FResourceData, FResources>(Resources, DeltaParms, *this);
	}
};

/** Specified to allow fast TArray replication */
template<>
struct TStructOpsTypeTraits<FResources> : public TStructOpsTypeTraitsBase2<FResources>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnResourceChangedSignature, const FGameplayTag&, ResourceTag, const int, Delta, const int, CurrentAmount);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BOXEL_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void ClearInventory();
	
	UFUNCTION(BlueprintCallable)
	void RemoveItem(AInventoryItem* Item);
	UFUNCTION(BlueprintCallable)
	bool AddItem(AInventoryItem* Item);
	
	UFUNCTION(BlueprintCallable)
	bool AddResource(const FGameplayTag& ResourceTag, const int Count);
	UFUNCTION(BlueprintCallable)
	bool ConsumeResource(const FGameplayTag& ResourceTag, const int Count);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetResourceCount(const FGameplayTag& ResourceTag) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetItemCount() const { return Items.Num(); }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AInventoryItem* GetItemByIndex(const int Index) const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetIndexOfItem(const AInventoryItem* Item) const;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	int MaxLargeItems = 1;
	UPROPERTY(EditDefaultsOnly)
	int MaxSmallItems = 1;
	UPROPERTY(EditDefaultsOnly)
	int MaxUtilityItems = 3;
	
	int GetMaxItemAmount(const EInventoryItem::Type& Type);
	
	UPROPERTY(Replicated)
	TArray<AInventoryItem*> Items;
	
	//TODO: See if just using a regular array fixes it, brings it down to either my data struct or the fast array itself
	UPROPERTY(ReplicatedUsing=OnRep_Resources)
	TArray<FResourceData> Resources;
	UFUNCTION()
	void OnRep_Resources(const TArray<FResourceData>& PreviousResources);
	
	//TODO: Optimization, use a map of FGameplayTags as the container for delegates, so that delegates only get the callback for
	//the tag they want, and not every tag. Should be fine for now with such small amounts of resources, but if we have a lot it will become an issue
	//GAS does the same thing for effect added/removed callbacks
	UPROPERTY(BlueprintAssignable)
	FOnResourceChangedSignature OnResourceChangedDelegate;
	
	//Gets the physical representation of the owner
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AController* GetOwnerController() const;
	
	static const UResourceDataAsset* GetResourceCDOFromTag(const FGameplayTag& Tag);
	
};
