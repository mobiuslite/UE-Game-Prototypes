// 


#include "SaveSystem/BoxelSaveSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "SaveSystem/BoxelSaveGame.h"

UBoxelSaveGame* UBoxelSaveSubsystem::GetGameSave()
{
	if (!GameSave)
	{
		GameSave = Cast<UBoxelSaveGame>(UGameplayStatics::CreateSaveGameObject(UBoxelSaveGame::StaticClass()));
	}

	return GameSave;
}

void UBoxelSaveSubsystem::SaveGame()
{
	if (UBoxelSaveGame* Save = GetGameSave())
	{
		UGameplayStatics::AsyncSaveGameToSlot(Save, SlotName, 0, nullptr);
	}
}

void UBoxelSaveSubsystem::LoadGame()
{
	if (UBoxelSaveGame* LoadedSave = Cast<UBoxelSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
	{
		GameSave = LoadedSave;
	}
}

float UBoxelSaveSubsystem::GetMouseSensitivity()
{
	return GetGameSave()->MouseSensitivity; 
}

void UBoxelSaveSubsystem::SetMouseSensitivity(const float& Value)
{
	GetGameSave()->MouseSensitivity = Value;
}

float UBoxelSaveSubsystem::GetFOV()
{
	return GetGameSave()->FOV;
}

void UBoxelSaveSubsystem::SetFOV(const float& Value)
{
	GetGameSave()->FOV = Value;
}