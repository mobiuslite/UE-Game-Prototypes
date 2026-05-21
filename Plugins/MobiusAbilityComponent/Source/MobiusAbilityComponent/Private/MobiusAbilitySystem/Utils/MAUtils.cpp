// 


#include "MobiusAbilitySystem/Utils/MAUtils.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
#include "MobiusAbilitySystem/Utils/MAGameplayTags.h"

FGameplayEffectSpecHandle UMAUtils::MakeHitDamageSpec(UAbilitySystemComponent* AbilityComponent,
                                                      UGameplayAbility* SourceAbility, const TSubclassOf<UGameplayEffect> GameplayEffectClass, const float DamageAmount, const FHitResult& HitResult, AActor* Causer)
{
	if (!AbilityComponent || !SourceAbility) return FGameplayEffectSpecHandle();
	if (!IsValid(GameplayEffectClass)) return FGameplayEffectSpecHandle();

	FGameplayEffectContextHandle Context = FGameplayEffectContextHandle(UAbilitySystemGlobals::Get().AllocGameplayEffectContext());
	Context.SetAbility(SourceAbility);

	const FGameplayAbilityActorInfo* ActorInfo = SourceAbility->GetCurrentActorInfo();
	
	if (ensure(ActorInfo))
	{
		Context.AddInstigator(ActorInfo->OwnerActor.Get(), Causer);
		Context.AddHitResult(HitResult);
		
		// Pass along the source object to the effect
		if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
		{
			if (FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(SourceAbility->GetCurrentAbilitySpecHandle()))
			{
				Context.AddSourceObject(AbilitySpec->SourceObject.Get());
			}
		}
	}
	
	FGameplayEffectSpecHandle Spec = AbilityComponent->MakeOutgoingSpec(GameplayEffectClass, 1, Context);
	Spec.Data->SetSetByCallerMagnitude(TAG_Params_DamageAmount, DamageAmount);
	
	return Spec;
}

FGameplayEffectSpecHandle UMAUtils::MakeSpecSetByCaller(const TSubclassOf<UGameplayEffect> GameplayEffectClass,
	const FGameplayTag& Tag, const float Value)
{
	const FGameplayEffectContextHandle Context = FGameplayEffectContextHandle(UAbilitySystemGlobals::Get().AllocGameplayEffectContext());
	
	FGameplayEffectSpecHandle Spec = FGameplayEffectSpecHandle
		(new FGameplayEffectSpec(GameplayEffectClass->GetDefaultObject<UGameplayEffect>(), Context, 1));
	Spec.Data->SetSetByCallerMagnitude(Tag, Value);
	
	return Spec;
}

bool UMAUtils::HasLooseGameplayTagEX(const AActor* Actor, const FGameplayTag& GameplayTag)
{
	if (const UAbilitySystemComponent* AbilitySysComp = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		return AbilitySysComp->HasMatchingGameplayTag(GameplayTag);
	}
	
	return false;
}

int UMAUtils::GetLooseGameplayTagCountEX(const AActor* Actor, const FGameplayTag& GameplayTag)
{
	if (const UAbilitySystemComponent* AbilitySysComp = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		return AbilitySysComp->GetGameplayTagCount(GameplayTag);
	}
	
	return 0;
}

bool UMAUtils::AddLooseGameplayTagEX(AActor* Actor, const FGameplayTag& GameplayTag, bool bAllowStacks,
                                     bool bShouldReplicate, int Count)
{
	if (UAbilitySystemComponent* AbilitySysComp = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
	{
		if (!bAllowStacks && AbilitySysComp->HasMatchingGameplayTag(GameplayTag))
		{
			return false;
		}
		
		AbilitySysComp->AddLooseGameplayTag(GameplayTag, Count, bShouldReplicate ? EGameplayTagReplicationState::CountToOwner : EGameplayTagReplicationState::None);
		return true;
	}

	return false;
}

bool UMAUtils::RemoveLooseGameplayTagEX(AActor* Actor, const FGameplayTag& GameplayTag,
	bool bShouldReplicate)
{
	if (UAbilitySystemComponent* AbilitySysComp = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
	{
		if (!AbilitySysComp->HasMatchingGameplayTag(GameplayTag))
		{
			return false;
		}
		
		AbilitySysComp->RemoveLooseGameplayTag(GameplayTag, 1, bShouldReplicate ? EGameplayTagReplicationState::CountToOwner : EGameplayTagReplicationState::None);
		return true;
	}

	return false;
}

void UMAUtils::Suicide(AActor* Actor, const bool bForce)
{
	if (UMobiusAbilitySystemComponent* AbilitySysComp = Cast<UMobiusAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor)))
	{
		AbilitySysComp->Suicide(bForce);
	}
}