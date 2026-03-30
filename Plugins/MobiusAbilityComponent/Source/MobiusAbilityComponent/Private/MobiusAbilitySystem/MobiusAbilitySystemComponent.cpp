#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
#include "MobiusAbilitySystem/Attributes/MACommonAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "MobiusAbilitySystem/MAGameplayAbility.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Invincible, "Gameplay.Invincible");

UMobiusAbilitySystemComponent::UMobiusAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UMobiusAbilitySystemComponent::AbilityLocalInputPressed(int32 InputID)
{
	InputsHeld.AddUnique(InputID);
	
	Super::AbilityLocalInputPressed(InputID);
}

void UMobiusAbilitySystemComponent::AbilityLocalInputReleased(int32 InputID)
{
	InputsHeld.Remove(InputID);
	Super::AbilityLocalInputReleased(InputID);
}

void UMobiusAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
	
	//Might move this to the character, makes the ASC more generic
	AttributeSet = GetSet<UMACommonAttributeSet>();
	AttributeSet->OnHealthChanged.AddDynamic(this, &ThisClass::OnAttributeSetHealthChanged);
}

void UMobiusAbilitySystemComponent::ProcessAbilityInput(const float DeltaTime, const bool bGamePaused)
{
	if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();
	
	//
	// Process all abilities that activate when the input is held.
	//
	for (const int32 InputID : InputsHeld)
	{
		for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
		{
			if (Spec.Ability && Spec.InputID == InputID && !Spec.IsActive())
			{
				const UMAGameplayAbility* AbilityCDO = Cast<UMAGameplayAbility>(Spec.Ability);
				if (AbilityCDO && AbilityCDO->GetActivationPolicy() == EMobiusAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(Spec.Handle);
				}
			}
		}
	}
	
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}
}

void UMobiusAbilitySystemComponent::ClearAbilityInput()
{
	InputsHeld.Empty();
}

void UMobiusAbilitySystemComponent::ResetAttributes()
{
	for (int32 i=0; i < DefaultStartingData.Num(); ++i)
	{
		if (DefaultStartingData[i].Attributes && DefaultStartingData[i].DefaultStartingTable)
		{
			UAttributeSet* Attributes = const_cast<UAttributeSet*>(GetOrCreateAttributeSubobject(DefaultStartingData[i].Attributes));
			Attributes->InitFromMetaDataTable(DefaultStartingData[i].DefaultStartingTable);
		}
	}
}

void UMobiusAbilitySystemComponent::OnAttributeSetHealthChanged(float EffectMagnitude, float OldValue, float NewValue)
{
	OnHealthChanged.Broadcast(EffectMagnitude, OldValue, NewValue);
}

void UMobiusAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

IAbilitySystemReplicationProxyInterface* UMobiusAbilitySystemComponent::GetReplicationInterface()
{
	if (const APlayerState* PlayerState = Cast<APlayerState>(GetOwner()))
	{
		if (IAbilitySystemReplicationProxyInterface* PlayerCharacter = Cast<IAbilitySystemReplicationProxyInterface>(PlayerState->GetPawn()))
		{
			return PlayerCharacter;
		}
	}

	return Super::GetReplicationInterface();
}

