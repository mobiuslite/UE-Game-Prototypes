// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MobiusGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDiedSignature, AActor*, DeadPlayer);

/**
 * 
 */
UCLASS()
class BOXEL_API AMobiusGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable)
	virtual void StartGame();
	
	UFUNCTION(BlueprintCallable)
	virtual void KillPlayer(AActor* Player);
	
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite)
	FOnPlayerDiedSignature OnPlayerDiedDelegate;
	
protected:
	
	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> CurrentAlivePlayers;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int NumBots = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<APawn> BotClass; 
};
