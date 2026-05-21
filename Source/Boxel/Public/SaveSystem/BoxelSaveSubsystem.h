// 

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BoxelSaveSubsystem.generated.h"

class UBoxelSaveGame;

UENUM(BlueprintType)
namespace EVolume
{
	
	enum Type : uint8
	{
		None,
	
		Master,
		Sfx,
		Vc,
	
		COUNT
	};
}


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
	
	//TODO: Add a single node to grab or set settings if possible, similar to how GetAttribute node works for GAS
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetMouseSensitivity();
	UFUNCTION(BlueprintCallable)
	void SetMouseSensitivity(const float& Value);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetVolume(const TEnumAsByte<EVolume::Type> Type);
	UFUNCTION(BlueprintCallable)
	void SetVolume(const TEnumAsByte<EVolume::Type> Type, const float& Volume);
	
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
