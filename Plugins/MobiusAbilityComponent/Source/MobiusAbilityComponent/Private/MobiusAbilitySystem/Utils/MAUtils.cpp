// 


#include "MobiusAbilitySystem/Utils/MAUtils.h"

#include "AbilitySystemComponent.h"

FGameplayEffectSpecHandle UMAUtils::MakeHitDamageSpec(UAbilitySystemComponent* AbilityComponent,
                                                      UGameplayAbility* SourceAbility, const TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	if (!AbilityComponent || !SourceAbility) return FGameplayEffectSpecHandle();
	if (!IsValid(GameplayEffectClass)) return FGameplayEffectSpecHandle();

	const FGameplayEffectContextHandle Context = SourceAbility->MakeEffectContext(SourceAbility->GetCurrentAbilitySpecHandle(), SourceAbility->GetCurrentActorInfo());
	
	return AbilityComponent->MakeOutgoingSpec(GameplayEffectClass, 1, Context);
}
