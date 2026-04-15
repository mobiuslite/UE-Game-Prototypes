// 

#pragma once

#include "CoreMinimal.h"
#include "DeathBringerGameMode.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/GameStateBase.h"
#include "DeathBringerGameState.generated.h"

UENUM()
namespace EDeathBringerRoundState
{
	enum Type : uint8
	{
		None,
	
		Preparing,
		Active,
		End,
	
		COUNT
	};
}

UCLASS()
class BOXEL_API ADeathBringerGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	void SetRoundState(const EDeathBringerRoundState::Type State);
	EDeathBringerRoundState::Type GetRoundState() const { return RoundState; }
	
	void SetRoundEndTime(const float EndTime);
	
	UFUNCTION(NetMulticast, Reliable)
	void ShowRoundEndToast(const bool bDeathBringerWin);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowRoundEndToast(const bool bDeathBringerWin);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	TEnumAsByte<EDeathBringerRoundState::Type> RoundState;
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	float RoundEndTimeWorldSeconds;
	
private:
	
	static ETeamAttitude::Type DeathBringerAttitudeSolver(FGenericTeamId A, FGenericTeamId B);
	
};
