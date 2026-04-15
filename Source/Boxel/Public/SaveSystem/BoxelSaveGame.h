// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BoxelSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class BOXEL_API UBoxelSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	
	UBoxelSaveGame();
	
	UPROPERTY(VisibleAnywhere)
	float MouseSensitivity;
	UPROPERTY(VisibleAnywhere)
	float FOV;
};
