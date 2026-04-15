// 

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BoxelSaveSubsystem.generated.h"

class UBoxelSaveGame;

UCLASS()
class BOXEL_API UBoxelSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UBoxelSaveGame* GetGameSave();
	
	UFUNCTION(BlueprintCallable)
	void SaveGame();
	UFUNCTION(BlueprintCallable)
	void LoadGame();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetMouseSensitivity();
	UFUNCTION(BlueprintCallable)
	void SetMouseSensitivity(const float& Value);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetFOV();
	UFUNCTION(BlueprintCallable)
	void SetFOV(const float& Value);
	
private:
	
	const FString SlotName = TEXT("BoxelSave");
	
	//Don't get this directly, use GetSaveGame instead
	UPROPERTY()
	UBoxelSaveGame* GameSave;
};
