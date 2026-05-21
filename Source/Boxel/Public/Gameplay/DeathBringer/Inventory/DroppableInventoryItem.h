// 

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Interfaces/InventoryItem.h"
#include "DroppableInventoryItem.generated.h"

UCLASS()
class BOXEL_API ADroppableInventoryItem : public AInventoryItem
{
	GENERATED_BODY()

public:
	ADroppableInventoryItem();
	
	UStaticMeshComponent* GetItemMesh() const { return ItemMesh; }
	
	virtual void OnRemovedFromInventory_Implementation(const UInventoryComponent* Inventory, AController* HolderController) override;

	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ItemMesh;
	
	UPROPERTY(EditDefaultsOnly, Category="Item|Throwing")
	float DropImpulseStrength = 240.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Item|Sound")
	USoundBase* DropSound;
	UPROPERTY(EditDefaultsOnly, Category="Item|Sound")
	USoundAttenuation* DropAttenuationSettings;
	UPROPERTY(EditDefaultsOnly, Category="Item|Sound")
	float HitSoundSpeedThreshold = 60.0f;
	UPROPERTY(EditDefaultsOnly, Category="Item|Sound")
	float MinTimeBetweenHitSound = 0.15f;
	
	virtual void SetPhysicsEnabled(const bool bEnabled) override;
	virtual void OnHolderHistoryAdded() override;
	virtual void OnHolderHistoryRemoved() override;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnItemHitGround();
private:
	
	float LastHitWorldTime;
	
	UFUNCTION()
	void OnItemMeshComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
