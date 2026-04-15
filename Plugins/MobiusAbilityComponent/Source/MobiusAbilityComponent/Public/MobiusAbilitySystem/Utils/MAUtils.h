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
	UFUNCTION(BlueprintCallable, Category="MAUtils|Ability")
	static FGameplayEffectSpecHandle MakeHitDamageSpec(UAbilitySystemComponent* AbilityComponent,
		UGameplayAbility* SourceAbility, const TSubclassOf<UGameplayEffect> GameplayEffectClass, const FHitResult& HitResult, AActor* Causer);
	
	UFUNCTION(BlueprintCallable, Category="MAUtils|Ability")
	static FGameplayEffectSpecHandle MakeSpecSetByCaller(const TSubclassOf<UGameplayEffect> GameplayEffectClass, const FGameplayTag& Tag, const float Value);
};
