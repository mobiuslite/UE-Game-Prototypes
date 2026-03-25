// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MAUtils.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class MOBIUSABILITYCOMPONENT_API UMAUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="BPUtils|Ability")
	static FGameplayEffectSpecHandle MakeHitDamageSpec(UAbilitySystemComponent* AbilityComponent,
		UGameplayAbility* SourceAbility, const TSubclassOf<UGameplayEffect> GameplayEffectClass);
};
