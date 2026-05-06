#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "GameplayCueManager.h"
#include "MobiusAbilitySystem/Attributes/MACommonAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "MobiusAbilitySystem/MAGameplayAbility.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Invincible, "Gameplay.Invincible");
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Forced, "Gameplay.Damage.Forced");
UE_DEFINE_GAMEPLAY_TAG(TAG_Resources_IgnoreCost, "Gameplay.Resources.IgnoreCost");


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

FGameplayAbilitySpecHandle UMobiusAbilitySystemComponent::K2_GiveAbility(TSubclassOf<UGameplayAbility> AbilityClass,
	int32 Level, int32 InputID, UObject* SourceObject)
{
	// build and validate the ability spec
	FGameplayAbilitySpec AbilitySpec = BuildAbilitySpecFromClass(AbilityClass, Level, InputID);
	AbilitySpec.SourceObject = SourceObject;

	// validate the class
	if (!IsValid(AbilitySpec.Ability))
	{
		ABILITY_LOG(Error, TEXT("K2_GiveAbility() called with an invalid Ability Class."));

		return FGameplayAbilitySpecHandle();
	}

	// grant the ability and return the handle. This will run validation and authority checks
	return GiveAbility(AbilitySpec);
}

void UMobiusAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}

void UMobiusAbilitySystemComponent::AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::WhileActive, GameplayCueParameters);
}

void UMobiusAbilitySystemComponent::RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters & GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
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
	
	const float CurrentHealth = GetNumericAttribute(UMACommonAttributeSet::GetCurrentHealthAttribute());
	OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth, CurrentHealth);
}

void UMobiusAbilitySystemComponent::Suicide(const bool bForce)
{
	if (!GetOwner()->HasAuthority()) return;
	if (HasMatchingGameplayTag(TAG_Gameplay_Invincible) && !bForce) return;
	
	const FGameplayEffectContextHandle Context = FGameplayEffectContextHandle(UAbilitySystemGlobals::Get().AllocGameplayEffectContext());
	const FGameplayEffectSpec Spec(SuicideGameplayEffectClass->GetDefaultObject<UGameplayEffect>(), Context, 0.0f);
	
	ApplyGameplayEffectSpecToSelf(Spec);
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

