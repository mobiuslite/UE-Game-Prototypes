// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Interfaces/InventoryItem.h"
#include "GunBase.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UGunGameplayAbility;

USTRUCT()
struct FHolderHistoryData
{
	GENERATED_BODY()
	
	UPROPERTY()
	const APawn* PreviousHolder;
	UPROPERTY()
	float HeldCooldownTimer;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedSignature, int, CurrentAmmo, int, MaxAmmo);

UCLASS()
class BOXEL_API AGunBase : public AActor, public IInventoryItem
{
	GENERATED_BODY()

public:
	AGunBase();
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnEquip(AController* HolderController) override;
	virtual void OnUnequip(AController* HolderController) override;
	
	virtual void OnAddedToInventory(const UInventoryComponent* Inventory, AController* HolderController) override;
	virtual void OnRemovedFromInventory(const UInventoryComponent* Inventory, AController* HolderController) override;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APawn* GetHolder () const { return Holder; }
	int GetAmmoCount () const { return CurrentClipAmmo; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool CanUseGun() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsLocallyHeldGun() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsDroppable() const { return bDroppable; }
	bool CanBePickedUp(const APawn* PawnHolder) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetFireRate() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float IsFullyAutomatic() const { return bFullyAutomatic; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FGameplayTag GetFireCueTag() const { return OnFireGameplayCueTag; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	USceneComponent* GetMuzzleComponent() const { return MuzzleLocation; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TSubclassOf<UAnimInstance> GetGunAnimInstanceClass() const { return GunABP; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TSubclassOf<UGameplayEffect> GetDamageEffectClass() const { return DamageClass; }
	
	void ApplySpread();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetTotalBulletSpread() const;
	
	void ConsumeAmmo();
	
	//Returns if reload was successful
	UFUNCTION(BlueprintNativeEvent)
	bool StartReload();
	
	void ResetGun();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* GunMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* MuzzleLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gun|Animation")
	TSubclassOf<UAnimInstance> GunABP;

	//TODO: Using RPM like this would allow clients to set their own RPM and make damage cheats through it
	UPROPERTY(EditDefaultsOnly, Category="Gun|Stats")
	float RPM = 400.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Stats")
	bool bFullyAutomatic;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gun|Stats", meta=(Units="Seconds"))
	float ReloadDuration = 1.75f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Throwing")
	bool bDroppable = true;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Throwing")
	float DropImpulseStrength = 240.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Throwing")
	float DropThrowOffset = 50.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gun|Ammo")
	FGameplayTag AmmoResourceTag;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Ammo")
	int AmmoPerClip = 20;
	UPROPERTY(Replicated, BlueprintReadOnly)
	int CurrentClipAmmo = 0;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy", meta=(Units="Degrees"))
	float SpreadPerBullet = 2.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy")
	float MovementSpreadMultiplier = 5.5f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy", meta=(Units="Degrees"))
	float MaxBulletSpread = 10.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy", meta=(Units="DegreesPerSecond"))
	float SpreadReductionPerSecond = 15.0f;
	float BulletSpreadAmount;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|Abilities")
	TSubclassOf<UGameplayAbility> GrantedAbilityClass;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Abilities")
	TSubclassOf<UGameplayEffect> DamageClass;
	UPROPERTY()
	FGameplayAbilitySpecHandle AbilityHandle;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|FXs")
	FGameplayTag OnFireGameplayCueTag;
	
	float ReloadTimer;
	UFUNCTION(BlueprintNativeEvent)
	void FinishedReloading();
	UFUNCTION(BlueprintNativeEvent)
	void CancelReloading();
	
	UFUNCTION(Server, Reliable)
	void Server_RequestReload();
	
	UPROPERTY(BlueprintAssignable)
	FOnAmmoChangedSignature OnAmmoChangedDelegate;
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnRep_Holder();
private:
	
	
	UPROPERTY(ReplicatedUsing=OnRep_Holder)
	APawn* Holder;
	
	void SetPhysicsEnabled(const bool bEnabled);
	
	//Stop players who just threw the gun from picking it up immediately 
	UPROPERTY()
	TArray<FHolderHistoryData> HolderHistory;
};
