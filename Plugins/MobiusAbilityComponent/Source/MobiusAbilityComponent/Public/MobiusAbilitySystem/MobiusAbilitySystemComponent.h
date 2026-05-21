#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "Attributes/MACommonAttributeSet.h"
#include "Utils/SuicideGameplayEffect.h"

#include "MobiusAbilitySystemComponent.generated.h"

USTRUCT(BlueprintType)
struct FStartingAbilityInfo
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayAbility> AbilityClass;
	UPROPERTY(EditAnywhere)
	int32 InputId = -1;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class MOBIUSABILITYCOMPONENT_API UMobiusAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UMobiusAbilitySystemComponent();
	
	virtual void AbilityLocalInputPressed(int32 InputID) override;
	virtual void AbilityLocalInputReleased(int32 InputID) override;
	virtual void ProcessAbilityInput(const float DeltaTime, const bool bGamePaused);
	virtual void ClearAbilityInput();
	
	FGameplayAbilitySpecHandle K2_GiveAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level= 0, int32 InputID = -1, UObject* SourceObject = nullptr);
	
	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
	
	UFUNCTION(BlueprintCallable)
	void ResetAttributes();
	
	void Suicide(const bool bForce);
	
	UPROPERTY(BlueprintAssignable)
	mutable FMAAttributeEvent OnHealthChanged;
	
	virtual void BeginPlay() override;
	
protected:
	
	UPROPERTY()
	TArray<int32> InputsHeld;
	
	UPROPERTY()
	TSubclassOf<UGameplayEffect> SuicideGameplayEffectClass = USuicideGameplayEffect::StaticClass();
	
	UPROPERTY(EditDefaultsOnly)
	TArray<FStartingAbilityInfo> StartingGrantedAbilities;
	
	UFUNCTION()
	void OnAttributeSetHealthChanged(float EffectMagnitude, float OldValue, float NewValue);
	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY()
	const UMACommonAttributeSet* AttributeSet;
	
	//Boiler Plate
public:
	virtual IAbilitySystemReplicationProxyInterface* GetReplicationInterface() override;
};
