// 

#pragma once

#include "CoreMinimal.h"
#include "GunGameplayAbility.h"
#include "HitscanGameplayAbility.generated.h"

class AGunBase;
/**
 * 
 */
UCLASS()
class BOXEL_API UHitscanGameplayAbility : public UGunGameplayAbility
{
	GENERATED_BODY()

protected:
	
	virtual void FireGun(AGunBase* Gun) override;
	virtual void OnFinishFire() override;
	
	void StartRangedWeaponTargeting();
	UFUNCTION()
	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);
	
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);
	
	TArray<FHitResult> PerformLocalTargeting();
	static int32 FindFirstPawnHitResult(const TArray<FHitResult>& HitResults);
	void TraceBulletsInCartridge(const FRangedWeaponFiringInput& InputData, OUT TArray<FHitResult>& OutHits);
	FHitResult DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const;
	// Does a single weapon trace, either sweeping or ray depending on if SweepRadius is above zero
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const;
	
	void AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const;
	
	
	
private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
};
