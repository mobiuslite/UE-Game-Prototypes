// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Gameplay/DeathBringer/Inventory/DroppableInventoryItem.h"
#include "GunBase.generated.h"

class UToastWidget;
class UGameplayEffect;
class UGameplayAbility;
class UGunGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedSignature, int, CurrentAmmo, int, MaxAmmo);

UCLASS()
class BOXEL_API AGunBase : public ADroppableInventoryItem
{
	GENERATED_BODY()

public:
	AGunBase();
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void OnEquip_Implementation(AController* HolderController) override;
	virtual void OnUnequip_Implementation(AController* HolderController) override;

	int GetAmmoCount () const { return CurrentClipAmmo; }
	float GetDamageAmount(const float Distance = 0) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool CanUseGun() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsLocallyHeldGun() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasScope() const { return bHasScope; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetEquipTime() const { return EquipTime; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetFireRate() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float IsFullyAutomatic() const { return bFullyAutomatic; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetGunZoomAmount(float& ZoomAmount) const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TSubclassOf<UToastWidget> GetAimWidgetClass() const { return AimToastClass; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FGameplayTag GetFireCueTag() const { return OnFireGameplayCueTag; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	USceneComponent* GetMuzzleComponent() const { return MuzzleLocation; }
	
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
	
	virtual void OnRep_Holder() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* MuzzleLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gun|Animation")
	TObjectPtr<UAnimMontage> FireMontage;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|Stats")
	float RPM = 400.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Stats")
	float EquipTime = 0.75f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Stats")
	bool bFullyAutomatic;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gun|Stats", meta=(Units="Seconds"))
	float ReloadDuration = 1.75f;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|Damage")
	float BaseDamageAmount = 20.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Damage")
	class UCurveFloat* DamageFalloff;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Damage")
	float GuiltyDamageMultiplier = 0.7f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gun|Aiming")
	bool bHasScope;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gun|Aiming")
	float ScopeZoomAmount = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gun|Ammo")
	FGameplayTag AmmoResourceTag;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Ammo")
	int AmmoPerClip = 20;
	UPROPERTY(Replicated, BlueprintReadOnly)
	int CurrentClipAmmo = 0;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy", meta=(Units="Degrees"))
	float MinSpreadAmount = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy", meta=(Units="Degrees", EditCondition = "bHasScope"))
	float AimingMinSpreadAmount = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy", meta=(Units="Degrees"))
	float SpreadPerBullet = 2.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy")
	float MovementSpreadMultiplier = 5.5f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy")
	float MovementSpreadEaseExp = 1.66;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy", meta=(Units="Degrees"))
	float MaxBulletSpread = 10.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy", meta=(Units="DegreesPerSecond"))
	float SpreadReductionPerSecond = 15.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|Accuracy", meta=(Units="Degrees"))
	float GuiltyAdditiveSpreadAmount = 3.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|Recoil", meta=(Units="Degrees"))
	float RecoilPerBullet = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Recoil", meta=(Units="Degrees"))
	float MaxRecoil = 2.5f;
	
	float BulletSpreadAmount;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|Abilities")
	TSubclassOf<UGameplayEffect> DamageClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|FXs")
	FGameplayTag OnFireGameplayCueTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|UI")
	TSubclassOf<UToastWidget> AimToastClass;
	
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
	
private:
	
	float EquipReadyWorldTime;
};
