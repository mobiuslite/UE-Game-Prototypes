// 


#include "MobiusAbilitySystem/Player/MAPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
#include "MobiusAbilitySystem/Player/MAPlayerState.h"

UAbilitySystemComponent* AMAPlayerController::GetAbilitySystemComponent() const
{
	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPlayerState<AMAPlayerState>());
}

void AMAPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UMobiusAbilitySystemComponent* AbilityComp = Cast<UMobiusAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		AbilityComp->ProcessAbilityInput(DeltaTime, bGamePaused);
	}
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AMAPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	
	if (UMobiusAbilitySystemComponent* AbilityComp = Cast<UMobiusAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		AbilityComp->ClearAbilityInput();
	}
}
