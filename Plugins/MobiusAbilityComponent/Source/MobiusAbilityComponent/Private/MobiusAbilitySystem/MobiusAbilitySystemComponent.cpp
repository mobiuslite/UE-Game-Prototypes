#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
#include "MobiusAbilitySystem/Attributes/MACommonAttributeSet.h"
#include "GameFramework/PlayerState.h"

UMobiusAbilitySystemComponent::UMobiusAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UMobiusAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
	
	//Might move this to the character, makes the ASC more generic
	AttributeSet = GetSet<UMACommonAttributeSet>();
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

