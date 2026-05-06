// 


#include "MobiusAbilitySystem/Utils/SuicideGameplayEffect.h"
#include "MobiusAbilitySystem/Attributes/MACommonAttributeSet.h"

USuicideGameplayEffect::USuicideGameplayEffect()
{
	const int32 Idx = Modifiers.Num();
	Modifiers.SetNum(Idx+1);
	FGameplayModifierInfo& Info = Modifiers[Idx];
	Info.ModifierMagnitude = FScalableFloat(999.0f);
	Info.ModifierOp = EGameplayModOp::Additive;
	Info.Attribute = UMACommonAttributeSet::GetDamageAmountAttribute();
	
	DurationPolicy = EGameplayEffectDurationType::Instant;
}
