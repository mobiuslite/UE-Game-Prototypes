// 


#include "MobiusAbilitySystem/Player/MAPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "MobiusAbilitySystem/Player/MAPlayerState.h"

UAbilitySystemComponent* AMAPlayerController::GetAbilitySystemComponent() const
{
	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPlayerState<AMAPlayerState>());
}
