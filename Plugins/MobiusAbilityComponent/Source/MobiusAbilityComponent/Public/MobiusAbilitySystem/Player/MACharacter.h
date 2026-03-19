#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemReplicationProxyInterface.h"
#include "GameFramework/Character.h"
#include "MACharacter.generated.h"

UCLASS()
class MOBIUSABILITYCOMPONENT_API AMACharacter : public ACharacter,
	public IAbilitySystemInterface,
	public IAbilitySystemReplicationProxyInterface
{
	GENERATED_BODY()

public:
	AMACharacter();
	AMACharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//Boilerplate
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void ForceReplication() override;
	
#pragma region GAMEPLAY_CUE_BOILERPLATE
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_InvokeGameplayCueExecuted_FromSpec(const FGameplayEffectSpecForRPC Spec, FPredictionKey PredictionKey) override;
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_InvokeGameplayCueExecuted(const FGameplayTag GameplayCueTag, FPredictionKey PredictionKey, FGameplayEffectContextHandle EffectContext) override;
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_InvokeGameplayCuesExecuted(const FGameplayTagContainer GameplayCueTags, FPredictionKey PredictionKey, FGameplayEffectContextHandle EffectContext) override;
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_InvokeGameplayCueExecuted_WithParams(const FGameplayTag GameplayCueTag, FPredictionKey PredictionKey, FGameplayCueParameters GameplayCueParameters) override;
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_InvokeGameplayCuesExecuted_WithParams(const FGameplayTagContainer GameplayCueTags, FPredictionKey PredictionKey, FGameplayCueParameters GameplayCueParameters) override;
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_InvokeGameplayCueAdded(const FGameplayTag GameplayCueTag, FPredictionKey PredictionKey, FGameplayEffectContextHandle EffectContext) override;
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_InvokeGameplayCueAdded_WithParams(const FGameplayTag GameplayCueTag, FPredictionKey PredictionKey, FGameplayCueParameters Parameters) override;
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_InvokeGameplayCueAddedAndWhileActive_FromSpec(const FGameplayEffectSpecForRPC& Spec, FPredictionKey PredictionKey) override;
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_InvokeGameplayCueAddedAndWhileActive_WithParams(const FGameplayTag GameplayCueTag, FPredictionKey PredictionKey, FGameplayCueParameters GameplayCueParameters) override;
	UFUNCTION(NetMulticast, Unreliable)
	virtual void NetMulticast_InvokeGameplayCuesAddedAndWhileActive_WithParams(const FGameplayTagContainer GameplayCueTags, FPredictionKey PredictionKey, FGameplayCueParameters GameplayCueParameters) override;
#pragma endregion
};
