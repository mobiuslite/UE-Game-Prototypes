// 


#include "MobiusAbilitySystem/Attributes/MACommonAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/PlayerState.h"
#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
#include "MobiusAbilitySystem/Player/MACharacter.h"
#include "MobiusAbilitySystem/Utils/MAGameplayTags.h"
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
		
		if (!TargetActor)
		{
			UE_LOG(LogTemp, Error, TEXT("Target actor is null when trying to apply damage"));
			return;
		}
		
		const FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
		
		FGameplayTagContainer EffectAssetTags;
		Data.EffectSpec.GetAllAssetTags(EffectAssetTags);
		
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
		
		AActor* SourceCauser = Context.GetEffectCauser();
		
		float FinalDamage = BaseDamageDone;
		if (const UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.Get())
		{
			FinalDamage *= PhysMat->DamageModifier.DamageThresholdMultiplier;
		}
		
		if (const UAbilitySystemComponent* TargetAbilityComponent = TargetActor->GetAbilitySystemComponent())
		{
			if (TargetAbilityComponent->HasMatchingGameplayTag(TAG_Gameplay_Invincible) && !EffectAssetTags.HasTagExact(TAG_Damage_Forced))
			{
				FinalDamage = 0.0f;
			}
		}
		
		if (FinalDamage > 0.0f)
		{	
			UE_LOG(LogTemp, Log, TEXT("Took Damage: %f"), FinalDamage);

			const float OldHealth = GetCurrentHealth();
			const float NewHealth = FMath::Clamp(OldHealth - FinalDamage, 0, GetMaxHealth());
			
			SetCurrentHealth(NewHealth);

			OnHealthChanged.Broadcast(-FinalDamage, OldHealth, NewHealth);
			
			const AController* Instigator = Cast<AController>(SourceInstigator);
			if (!Instigator)
			{
				UE_LOG(LogTemp, Error, TEXT("MACommonAttributeSet: Instigator wasn't a controller or player state, PLEASE FIX"))
			}
				
			const bool bIsDead = NewHealth <= 0.0f;
			TargetActor->Client_OnDamageTaken(Instigator, SourceCauser, bIsDead);
			if (bIsDead)
			{
				TargetActor->Server_OnPlayerDead(HitResult, Instigator, SourceCauser);
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealAmountAttribute())
	{
		const float BaseHealAmount = GetHealAmount();
		SetHealAmount(0.0f);
		
		AMACharacter* TargetActor = nullptr;
		if (Data.Target.AbilityActorInfo.IsValid())
		{
			TargetActor = Cast<AMACharacter>(Data.Target.AbilityActorInfo->AvatarActor.Get());
		}
		
		if (!TargetActor)
		{
			UE_LOG(LogTemp, Error, TEXT("Target actor is null when trying to apply healing"));
			return;
		}
		
		if (BaseHealAmount > 0.0f)
		{	
			UE_LOG(LogTemp, Log, TEXT("Healed amount: %f"), BaseHealAmount);

			const float OldHealth = GetCurrentHealth();
			const float NewHealth = FMath::Clamp(OldHealth + BaseHealAmount, 0, GetMaxHealth());
			
			SetCurrentHealth(NewHealth);

			if (!FMath::IsNearlyEqual(OldHealth, NewHealth))
			{
				OnHealthChanged.Broadcast(BaseHealAmount, OldHealth, NewHealth);
			}
		}
	}
	
	Super::PostGameplayEffectExecute(Data);
}

void UMACommonAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetMaxHealthAttribute())
	{
		const float OldMaxHealth = GetMaxHealth();
		const float OldCurrentHealth = GetCurrentHealth();
		const float Delta = NewValue - OldMaxHealth;

		const float NewHealthValue = FMath::Clamp(GetCurrentHealth() + Delta, 0, NewValue);
		SetCurrentHealth(NewHealthValue);
		
		OnHealthChanged.Broadcast(Delta, OldCurrentHealth, NewHealthValue);
	}
}

void UMACommonAttributeSet::OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, CurrentHealth, OldCurrentHealth);
	
	const float Health = GetCurrentHealth();
	const float EstimatedMagnitude = Health - OldCurrentHealth.GetCurrentValue();
	
	OnHealthChanged.Broadcast(EstimatedMagnitude, OldCurrentHealth.GetCurrentValue(), Health);
}

void UMACommonAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxHealth, OldMaxHealth);
	
	//Fixes a bug where OnHealthChanged won't get called if only max health changed and not current health
	const float Health = GetCurrentHealth();
	OnHealthChanged.Broadcast(0.0f, Health, Health);
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
