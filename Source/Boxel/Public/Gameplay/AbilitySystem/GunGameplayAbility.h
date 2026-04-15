// 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MobiusAbilitySystem/MAGameplayAbility.h"
#include "GunGameplayAbility.generated.h"

class AGunBase;
/**
 * 
 */
UCLASS()
class BOXEL_API UGunGameplayAbility : public UMAGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	UGunGameplayAbility();
	
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual void FireGun(AGunBase* Gun) {};
	virtual void OnFinishFire() {};
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AGunBase* GetGunActor() const;
	
	struct FRangedWeaponFiringInput
	{
		// Start of the trace
		FVector StartTrace;

		// End of the trace if aim were perfect
		FVector EndTrace;

		// The direction of the trace if aim were perfect
		FVector AimDir;

		FRangedWeaponFiringInput()
			: StartTrace(ForceInitToZero)
			, EndTrace(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};
	
	UPROPERTY()
	FTimerHandle TimerHandle;
	UFUNCTION()
	void OnAutoTimerComplete();
};
