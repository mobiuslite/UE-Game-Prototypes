// 


#include "Core/BoxelGameInstance.h"

#include "SteamToolsSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UBoxelGameInstance::Init()
{
	Super::Init();
	if (USteamToolsSubsystem* SteamSubsystem = GetSubsystem<USteamToolsSubsystem>())
	{
		const FSteamworksInitializationResponse Response = SteamSubsystem->InitializeSteamAPI(GetWorld());
		if (Response.bSuccess)
		{
			UE_LOG(LogTemp, Display, TEXT("Steam init success!"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Steam init failed!!!"));
		}
		
		SteamSubsystem->SteamRichPresenceJoinRequested.AddDynamic(this, &ThisClass::OnJoinRequested);
	}
}

void UBoxelGameInstance::OnJoinRequested(int64 SteamID, const FString& ConnectionString)
{
	FString URL = "steam.";
	URL.Append(ConnectionString);
	
	UGameplayStatics::OpenLevel(GetWorld(), *URL);
}
