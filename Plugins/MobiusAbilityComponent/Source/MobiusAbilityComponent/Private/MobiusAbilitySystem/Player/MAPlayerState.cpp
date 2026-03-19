#include "MobiusAbilitySystem/Player/MAPlayerState.h"

#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"

AMAPlayerState::AMAPlayerState()
{
	AbilityComponent = CreateDefaultSubobject<UMobiusAbilitySystemComponent>(TEXT("Mobius Ability Component"));
	AbilityComponent->SetIsReplicated(true);
	AbilityComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	//What Lyra uses
	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* AMAPlayerState::GetAbilitySystemComponent() const
{
	return AbilityComponent;
}
