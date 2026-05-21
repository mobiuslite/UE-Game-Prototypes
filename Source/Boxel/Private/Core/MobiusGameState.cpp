// 


#include "Core/MobiusGameState.h"

void AMobiusGameState::BroadcastRoundReset_Implementation()
{
	PreRoundHardResetDelegate.Broadcast();
	OnRoundHardResetDelegate.Broadcast();
}
