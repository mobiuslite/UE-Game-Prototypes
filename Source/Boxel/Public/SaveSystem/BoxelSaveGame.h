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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float MouseSensitivity;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float FOV;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float MasterVolume;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float SFXVolume;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float VCVolume;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bCameraMotion;
};
