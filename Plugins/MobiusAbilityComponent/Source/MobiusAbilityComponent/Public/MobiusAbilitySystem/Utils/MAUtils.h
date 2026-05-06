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
	
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="MAUtils|Ability", meta=(DefaultToSelf = "Actor"))
	static bool HasLooseGameplayTagEX(AActor* Actor, const FGameplayTag& GameplayTag);
	
	//Returns true if successful in adding or removing. Returns false if failed for any reason, including tag not existing to remove
	UFUNCTION(BlueprintCallable, Category="MAUtils|Ability", meta=(DefaultToSelf = "Actor"))
	static bool AddLooseGameplayTagEX(AActor* Actor, const FGameplayTag& GameplayTag, bool bAllowStacks, bool bShouldReplicate);
	UFUNCTION(BlueprintCallable, Category="MAUtils|Ability", meta=(DefaultToSelf = "Actor"))
	static bool RemoveLooseGameplayTagEX(AActor* Actor, const FGameplayTag& GameplayTag, bool bShouldReplicate);
	
	UFUNCTION(BlueprintCallable, Category="MAUtils|Ability", meta=(DefaultToSelf = "Actor"))
	static void Suicide(AActor* Actor, const bool bForce = false);
};
