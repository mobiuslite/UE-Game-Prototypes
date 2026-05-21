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

float UBoxelSaveSubsystem::GetVolume(const TEnumAsByte<EVolume::Type> Type)
{
	float Result = 0.0f;
	
	switch (Type) 
	{
	case EVolume::Master:
		{
			Result = GetGameSave()->MasterVolume;
		}
		break;
	case EVolume::Sfx:
		{
			Result = GetGameSave()->SFXVolume;
		}
		break;
	case EVolume::Vc:
		{
			Result = GetGameSave()->VCVolume;
		}
		break;
	default: ;
	}
	
	return Result;
}

void UBoxelSaveSubsystem::SetVolume(const TEnumAsByte<EVolume::Type> Type, const float& Volume)
{
	switch (Type) 
	{
	case EVolume::Master:
		{
			GetGameSave()->MasterVolume = Volume;
		}
		break;
	case EVolume::Sfx:
		{
			GetGameSave()->SFXVolume = Volume;
		}
		break;
	case EVolume::Vc:
		{
			GetGameSave()->VCVolume = Volume;
		}
		break;
	default:
		{
			UE_LOG(LogTemp, Warning, TEXT("Setting Volume failed, wrong type given: %d"), Type.GetValue())
		}
		break;
	}
}

float UBoxelSaveSubsystem::GetFOV()
{
	return GetGameSave()->FOV;
}

void UBoxelSaveSubsystem::SetFOV(const float& Value)
{
	GetGameSave()->FOV = Value;
}