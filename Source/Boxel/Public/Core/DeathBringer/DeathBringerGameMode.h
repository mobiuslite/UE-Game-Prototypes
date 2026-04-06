// 

#pragma once

#include "CoreMinimal.h"
#include "../MobiusGameMode.h"
#include "DeathBringerGameMode.generated.h"

enum EDeathBringerRoundState : int;
class ADeathBringerGameState;

/*
	MustSpectate is overridden in the BP class to make players joining while game is active force to spectate. 
	For some reason you can't override any of the spawn to spectate code in c++ afaik...

*/

UCLASS()
class BOXEL_API ADeathBringerGameMode : public AMobiusGameMode
{
	GENERATED_BODY()
	
public:
	
	ADeathBringerGameMode();
	
	virtual void Tick(float DeltaSeconds) override;
	
	//Hard reset will delete all bodies, reset all guns, and reset all players
	void PrepareGame(const bool bHardReset);
	
	virtual void StartGame() override;
	
	UFUNCTION(BlueprintNativeEvent)
	void EndDeathBringerGame(const bool bDeathBringerWin);
	
	virtual void KillPlayer(APawn* Player) override;
	
	static constexpr int32 DEATHBRINGER_TEAMID = 2;
	static constexpr int32 NORMALPLAYER_TEAMID = 1;
	static constexpr int32 NOTEAM_TEAMID = 255;
	
protected:
	
	virtual void BeginPlay() override;
	
	void SetRoundState(const EDeathBringerRoundState RoundState);
	EDeathBringerRoundState GetRoundState() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DeathBringerPlayerRatio = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MinDeathBringers = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MinPlayers = 3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Units="Seconds"))
	float StartGameWaitTime = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Units="Seconds"))
	float EndGameWaitTime = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Units="Minutes"))
	float GameLengthMinutes = 5.0f;
	
	float GameTimer;
	
	bool CanBeDeathBringer(const APlayerController* Controller) const;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<APawn*> DeathBringers;
	
	virtual void OnPlayerLogin(AGameModeBase* GameMode, APlayerController* PC) override;
	virtual void OnPlayerLogout(AGameModeBase* GameMode, AController* PC) override;
};

