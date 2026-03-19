

#include "Gameplay/Player/BoxelPlayerState.h"
#include "Gameplay/Player/BoxelPlayerController.h"

void ABoxelPlayerState::OnSetSpeaking_Implementation(const bool bSpeaking)
{
	OnSpeakingChanged.Broadcast(bSpeaking);
	if (const ABoxelPlayerController* BoxelController = Cast<ABoxelPlayerController>(GetPlayerController()))
	{
		BoxelController->OnSpeakingChanged.Broadcast(bSpeaking);
	}
}

void ABoxelPlayerState::SetSpeaking(const bool bSpeaking)
{
	OnSetSpeaking(bSpeaking);
}
