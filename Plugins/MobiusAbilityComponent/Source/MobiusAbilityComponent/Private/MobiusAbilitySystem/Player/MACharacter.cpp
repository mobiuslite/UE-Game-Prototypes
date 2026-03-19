
#include "MobiusAbilitySystem/Player/MACharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
#include "MobiusAbilitySystem/Player/MAPlayerState.h"


AMACharacter::AMACharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

AMACharacter::AMACharacter(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMACharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMACharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
	{
		// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
		if (UAbilitySystemComponent* AbilityComp = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(this))
		{
			AbilityComp->InitAbilityActorInfo(PS, this);
		}
	}
}

void AMACharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
	{
		// Set the ASC for clients. Server does this in PossessedBy.
		if (UAbilitySystemComponent* AbilityComp = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(this))
		{
			AbilityComp->InitAbilityActorInfo(PS, this);
		}
	}
}

void AMACharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

#pragma region GAMEPLAY_CUE_BOILERPLATE

UAbilitySystemComponent* AMACharacter::GetAbilitySystemComponent() const
{
	const AMAPlayerState* MAPlayerState = GetPlayerState<AMAPlayerState>();
	if (!MAPlayerState) return nullptr;
	
	return MAPlayerState->GetAbilitySystemComponent();
}

void AMACharacter::ForceReplication()
{
	ForceNetUpdate();
}


void AMACharacter::NetMulticast_InvokeGameplayCuesAddedAndWhileActive_WithParams_Implementation(
	const FGameplayTagContainer GameplayCueTags, FPredictionKey PredictionKey,
	FGameplayCueParameters GameplayCueParameters)
{
	if (HasAuthority() || PredictionKey.IsLocalClientKey() == false)
	{
		for (const FGameplayTag& GameplayCueTag : GameplayCueTags)
		{
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				ASC->InvokeGameplayCueEvent(GameplayCueTag, EGameplayCueEvent::OnActive, GameplayCueParameters);
				ASC->InvokeGameplayCueEvent(GameplayCueTag, EGameplayCueEvent::WhileActive, GameplayCueParameters);
			}
		}
	}
}

void AMACharacter::NetMulticast_InvokeGameplayCueAddedAndWhileActive_WithParams_Implementation(
	const FGameplayTag GameplayCueTag, FPredictionKey PredictionKey, FGameplayCueParameters GameplayCueParameters)
{
	if (HasAuthority() || PredictionKey.IsLocalClientKey() == false)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->InvokeGameplayCueEvent(GameplayCueTag, EGameplayCueEvent::OnActive, GameplayCueParameters);
			ASC->InvokeGameplayCueEvent(GameplayCueTag, EGameplayCueEvent::WhileActive, GameplayCueParameters);
		}
	}
}

void AMACharacter::NetMulticast_InvokeGameplayCueAddedAndWhileActive_FromSpec_Implementation(
	const FGameplayEffectSpecForRPC& Spec, FPredictionKey PredictionKey)
{
	if (HasAuthority() || PredictionKey.IsLocalClientKey() == false)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->InvokeGameplayCueEvent(Spec, EGameplayCueEvent::OnActive);
			ASC->InvokeGameplayCueEvent(Spec, EGameplayCueEvent::WhileActive);
		}
	}
}

void AMACharacter::NetMulticast_InvokeGameplayCueAdded_WithParams_Implementation(
	const FGameplayTag GameplayCueTag, FPredictionKey PredictionKey, FGameplayCueParameters Parameters)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		const bool bIsMixedReplicationFromServer = (ASC->ReplicationMode == EGameplayEffectReplicationMode::Mixed && PredictionKey.IsServerInitiatedKey() && IsLocallyControlled());
		if (HasAuthority() || (PredictionKey.IsLocalClientKey() == false && !bIsMixedReplicationFromServer))
		{
			ASC->InvokeGameplayCueEvent(GameplayCueTag, EGameplayCueEvent::OnActive, Parameters);
		}
	}
}

void AMACharacter::NetMulticast_InvokeGameplayCueAdded_Implementation(const FGameplayTag GameplayCueTag,
                                                                            FPredictionKey PredictionKey, FGameplayEffectContextHandle EffectContext)
{
	if (HasAuthority() || PredictionKey.IsLocalClientKey() == false)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->InvokeGameplayCueEvent(GameplayCueTag, EGameplayCueEvent::OnActive, EffectContext);
		}
	}
}

void AMACharacter::NetMulticast_InvokeGameplayCuesExecuted_WithParams_Implementation(
	const FGameplayTagContainer GameplayCueTags, FPredictionKey PredictionKey,
	FGameplayCueParameters GameplayCueParameters)
{
	if (HasAuthority() || PredictionKey.IsLocalClientKey() == false)
	{
		for (const FGameplayTag& GameplayCueTag : GameplayCueTags)
		{
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				ASC->InvokeGameplayCueEvent(GameplayCueTag, EGameplayCueEvent::Executed, GameplayCueParameters);
			}
		}
	}
}

void AMACharacter::NetMulticast_InvokeGameplayCueExecuted_WithParams_Implementation(
	const FGameplayTag GameplayCueTag, FPredictionKey PredictionKey, FGameplayCueParameters GameplayCueParameters)
{
	if (HasAuthority() || PredictionKey.IsLocalClientKey() == false)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->InvokeGameplayCueEvent(GameplayCueTag, EGameplayCueEvent::Executed, GameplayCueParameters);
		}
	}
}

void AMACharacter::NetMulticast_InvokeGameplayCuesExecuted_Implementation(
	const FGameplayTagContainer GameplayCueTags, FPredictionKey PredictionKey,
	FGameplayEffectContextHandle EffectContext)
{
	if (HasAuthority() || PredictionKey.IsLocalClientKey() == false)
	{
		for (const FGameplayTag& GameplayCueTag : GameplayCueTags)
		{
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				ASC->InvokeGameplayCueEvent(GameplayCueTag, EGameplayCueEvent::Executed, EffectContext);
			}
		}
	}
}

void AMACharacter::NetMulticast_InvokeGameplayCueExecuted_Implementation(const FGameplayTag GameplayCueTag,
                                                                               FPredictionKey PredictionKey, FGameplayEffectContextHandle EffectContext)
{
	if (HasAuthority() || PredictionKey.IsLocalClientKey() == false)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->InvokeGameplayCueEvent(GameplayCueTag, EGameplayCueEvent::Executed, EffectContext);
		}
	}
}

void AMACharacter::NetMulticast_InvokeGameplayCueExecuted_FromSpec_Implementation(
	const FGameplayEffectSpecForRPC Spec, FPredictionKey PredictionKey)
{
	if (HasAuthority() || PredictionKey.IsLocalClientKey() == false)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->InvokeGameplayCueEvent(Spec, EGameplayCueEvent::Executed);
		}
	}
}
#pragma endregion 