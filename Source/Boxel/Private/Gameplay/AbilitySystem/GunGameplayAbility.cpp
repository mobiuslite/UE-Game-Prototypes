// 


#include "Gameplay/AbilitySystem/GunGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "Gameplay/Weapons/GunBase.h"

UGunGameplayAbility::UGunGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EMobiusAbilityActivationPolicy::WhileInputActive;
}

void UGunGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AGunBase* Gun = GetGunActor();
	if (!Gun)
	{			
		constexpr bool bReplicateEndAbility = true;
		constexpr bool bWasCancelled = true;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
	
	FireGun(Gun);
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::OnAutoTimerComplete, Gun->GetFireRate(), false);
}

void UGunGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogTemp, Log, TEXT("Ended Ability"));
	
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}

		OnFinishFire();

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

//TODO: Make Semi auto only fire after waiting the timer and input is released
void UGunGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	K2_EndAbility();
}

void UGunGameplayAbility::OnAutoTimerComplete()
{
	if (const AGunBase* Gun = GetGunActor())
	{
		if (Gun->IsFullyAutomatic())
		{
			K2_EndAbility();
		}
	}
}

AGunBase* UGunGameplayAbility::GetGunActor() const
{
	const APlayerState* OwningActor = Cast<APlayerState>(GetOwningActorFromActorInfo());
	if (!OwningActor) return nullptr;
	
	const ABoxelPlayerCharacter* BoxelPlayer = OwningActor->GetPawn<ABoxelPlayerCharacter>();
	if (!BoxelPlayer) return nullptr;
	
	return BoxelPlayer->GetHeldGun();
}