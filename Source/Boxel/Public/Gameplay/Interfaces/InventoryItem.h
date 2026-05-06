// 

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "UObject/Interface.h"
#include "InventoryItem.generated.h"


class UGameplayEffect;
class UGameplayAbility;
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

USTRUCT()
struct FHolderHistoryData
{
	GENERATED_BODY()
	
	UPROPERTY()
	const APawn* PreviousHolder;
	UPROPERTY()
	float HeldCooldownTimer;
};

UCLASS()
class BOXEL_API AInventoryItem : public AActor
{
	GENERATED_BODY()

public:
	
	AInventoryItem();
	
	//When item is put in hands
	//These should only be called by local or authority, simulated proxies should not call these
	UFUNCTION(BlueprintNativeEvent)
	void OnEquip(AController* HolderController);
	UFUNCTION(BlueprintNativeEvent)
	void OnUnequip(AController* HolderController);
	
	//To override, use _Implementation override function
	UFUNCTION(BlueprintNativeEvent)
	void OnAddedToInventory(const UInventoryComponent* Inventory, AController* HolderController);
	UFUNCTION(BlueprintNativeEvent)
	void OnRemovedFromInventory(const UInventoryComponent* Inventory, AController* HolderController);
	
	virtual bool CanBePickedUp(const APawn* PawnHolder) const;
	virtual bool CanBeDropped() const { return bCanBeDropped; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsHeldInHand() const { return bVisible; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TEnumAsByte<EInventoryItem::Type> GetItemType() const { return ItemType; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TSubclassOf<UAnimInstance> GetHeldABPClass() const { return HeldABP; }
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	
	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<EInventoryItem::Type> ItemType;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> EquippedGrantedAbilityClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> EquippedGrantedEffectClass;
	UPROPERTY(EditDefaultsOnly)
	bool bCanBeDropped = true;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAnimInstance> HeldABP;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APawn* GetHolder() const { return HolderPrivate; }
	UFUNCTION()
	virtual void OnRep_Holder();
	
	virtual void SetPhysicsEnabled(const bool bEnabled);
	
	//Holder history are call backs that happen when a player drops a weapon, and is cleared after 0.5 seconds
	//This is useful for stopping collision between the previous holder and the item, so they don't immediately attempt to pick up the item again
	virtual void OnHolderHistoryAdded() {};
	virtual void OnHolderHistoryRemoved() {};
	
private:
	UPROPERTY(ReplicatedUsing=OnRep_Visible)
	bool bVisible = true;
	UFUNCTION()
	void OnRep_Visible();
	
	UPROPERTY(ReplicatedUsing=OnRep_Holder)
	APawn* HolderPrivate;
	
	UPROPERTY()
	TArray<FHolderHistoryData> HolderHistory;
	
	UPROPERTY()
	FGameplayAbilitySpecHandle AbilityHandle;
	UPROPERTY()
	FActiveGameplayEffectHandle EffectHandle;
};
