// 


#include "Core/MobiusGameMode.h"

#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/MobiusUtils.h"

void AMobiusGameMode::StartGame()
{
	AlivePlayers.Empty();
	
	TArray<APawn*> PlayerActors = UMobiusUtils::GetAllActorsOfClassEX<APawn>(GetWorld(), ABoxelPlayerCharacter::StaticClass());
	
	if (IsValid(BotClass))
	{
		for (int i = 0; i < NumBots; ++i)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			APawn* NewBot = GetWorld()->SpawnActor<APawn>(BotClass, Params);
			PlayerActors.Add(NewBot);	
		}
	}
	
	AlivePlayers.Append(PlayerActors);
}

void AMobiusGameMode::KillPlayer(APawn* Player)
{
	AlivePlayers.Remove(Player);
	OnPlayerDiedDelegate.Broadcast(Player->GetController());
}

void AMobiusGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	FGameModeEvents::OnGameModePostLoginEvent().AddUObject(this, &ThisClass::OnPlayerLogin);
	FGameModeEvents::OnGameModeLogoutEvent().AddUObject(this, &ThisClass::OnPlayerLogout);
	
	//OnPlayerLogin doesn't get called for the server owner, but we still want them to go through the logic
	if (APlayerController* ServerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		OnPlayerLogin(this, ServerController);
	}
}
