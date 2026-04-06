// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DeathBringerGameState.generated.h"

UENUM()
enum EDeathBringerRoundState
{
	None,
	
	Preparing,
	Active,
	End,
	
	COUNT
};

UCLASS()
class BOXEL_API ADeathBringerGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	void SetRoundState(const EDeathBringerRoundState State);
	EDeathBringerRoundState GetRoundState() const { return RoundState; }
	
	void SetRoundEndTime(const float EndTime);
	
	UFUNCTION(NetMulticast, Reliable)
	void ShowRoundEndToast(const bool bDeathBringerWin);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowRoundEndToast(const bool bDeathBringerWin);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UPROPERTY(BlueprintReadOnly, Replicated)
	TEnumAsByte<EDeathBringerRoundState> RoundState;
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	float RoundEndTimeWorldSeconds;
};
