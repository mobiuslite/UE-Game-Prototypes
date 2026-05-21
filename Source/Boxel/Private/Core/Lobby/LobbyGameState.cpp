// 


#include "Core/Lobby/LobbyGameState.h"

void ALobbyGameState::AddKillHistory_Implementation(const APlayerState* KillInstigator, const APlayerState* KilledPlayer)
{
	BP_AddKillHistory(KillInstigator, KilledPlayer);
}
