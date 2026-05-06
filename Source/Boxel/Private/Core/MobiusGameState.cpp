// 


#include "Core/MobiusGameState.h"

void AMobiusGameState::BroadcastRoundReset_Implementation()
{
	OnRoundHardResetDelegate.Broadcast();
}
