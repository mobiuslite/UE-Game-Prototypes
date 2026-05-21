// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MobiusGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundHardResetSignature);

UCLASS()
class BOXEL_API AMobiusGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(NetMulticast, Reliable)
	void BroadcastRoundReset();
	
	//Game modes need to implement this manually. Not automatically called
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite)
	FOnRoundHardResetSignature OnRoundHardResetDelegate;
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite)
	FOnRoundHardResetSignature PreRoundHardResetDelegate;
};
