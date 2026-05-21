// 

#pragma once

#include "CoreMinimal.h"
#include "Core/MobiusGameState.h"
#include "LobbyGameState.generated.h"

UCLASS()
class BOXEL_API ALobbyGameState : public AMobiusGameState
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(NetMulticast, Unreliable)
	void AddKillHistory(const APlayerState* KillInstigator, const APlayerState* KilledPlayer);
	
	UFUNCTION(BlueprintImplementableEvent)
	void BP_AddKillHistory(const APlayerState* KillInstigator, const APlayerState* KilledPlayer);
};
