// 


#include "MobiusAbilitySystem/Attributes/MACommonAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/PlayerState.h"
#include "MobiusAbilitySystem/Player/MACharacter.h"
#include "Net/UnrealNetwork.h"

void UMACommonAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetDamageAmountAttribute())
	{
		const float BaseDamageDone = GetDamageAmount();
		SetDamageAmount(0.0f);
		
		AMACharacter* TargetActor = nullptr;
		if (Data.Target.AbilityActorInfo.IsValid())
		{
			TargetActor = Cast<AMACharacter>(Data.Target.AbilityActorInfo->AvatarActor.Get());
		}
		
		const FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
		
		FHitResult HitResult;
		if (const FHitResult* HitPtr = Context.GetHitResult())
		{
			HitResult = *HitPtr;
		}
		
		const AActor* SourceInstigator = Context.GetInstigator();
		if (const APlayerState* PlayerState = Cast<APlayerState>(SourceInstigator))
		{
			SourceInstigator = PlayerState->GetPlayerController();
		}
		
		if (BaseDamageDone > 0.0f)
		{	
			UE_LOG(LogTemp, Log, TEXT("Took Damage: %f"), BaseDamageDone);

			const float OldHealth = GetCurrentHealth();
			const float NewHealth = FMath::Clamp(OldHealth - BaseDamageDone, 0, GetMaxHealth());
			
			SetCurrentHealth(NewHealth);

			if (IsValid(TargetActor))
			{
				const AController* Instigator = Cast<AController>(SourceInstigator);
				if (!Instigator)
				{
					UE_LOG(LogTemp, Error, TEXT("MACommonAttributeSet: Instigator wasn't a controller or player state, PLEASE FIX"))
				}
				
				//TargetActor->OnTookDamage(BaseDamageDone, Instigator, HitResult);
			}
		}
	}
	
	
	Super::PostGameplayEffectExecute(Data);
}

void UMACommonAttributeSet::OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, CurrentHealth, OldCurrentHealth);
}

void UMACommonAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxHealth, OldMaxHealth);
}

void UMACommonAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MoveSpeed, OldMoveSpeed);
}

void UMACommonAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, AttackSpeed, OldAttackSpeed);
}

void UMACommonAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, CurrentHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxHealth, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, AttackSpeed, COND_None, REPNOTIFY_Always);
}
