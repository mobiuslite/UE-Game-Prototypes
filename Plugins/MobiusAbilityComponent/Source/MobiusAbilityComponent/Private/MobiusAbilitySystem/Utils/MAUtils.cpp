// 


#include "MobiusAbilitySystem/Utils/MAUtils.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

FGameplayEffectSpecHandle UMAUtils::MakeHitDamageSpec(UAbilitySystemComponent* AbilityComponent,
                                                      UGameplayAbility* SourceAbility, const TSubclassOf<UGameplayEffect> GameplayEffectClass, const FHitResult& HitResult, AActor* Causer)
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
	
	return AbilityComponent->MakeOutgoingSpec(GameplayEffectClass, 1, Context);
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
