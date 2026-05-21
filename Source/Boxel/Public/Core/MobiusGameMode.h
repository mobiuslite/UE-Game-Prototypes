// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MobiusGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDiedSignature, AController*, DeadPlayerController);

UCLASS()
class BOXEL_API AMobiusGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable)
	virtual void StartGame();
	
	UFUNCTION(BlueprintCallable)
	virtual void KillPlayer(APawn* Player, const AController* KilledBy);
	
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite)
	FOnPlayerDiedSignature OnPlayerDiedDelegate;
protected:
	
	virtual void OnPlayerLogin(AGameModeBase* GameMode, APlayerController* PC) {}
	virtual void OnPlayerLogout(AGameModeBase* GameMode, AController* PC) {}
	
	UPROPERTY(BlueprintReadOnly)
	TArray<APawn*> AlivePlayers;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int NumBots = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<APawn> BotClass; 
	
	virtual void BeginPlay() override;
};
