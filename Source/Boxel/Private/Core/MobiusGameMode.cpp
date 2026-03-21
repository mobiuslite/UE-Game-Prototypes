// 


#include "Core/MobiusGameMode.h"

#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void AMobiusGameMode::StartGame()
{
	CurrentAlivePlayers.Empty();
	
	TArray<AActor*> PlayerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoxelPlayerCharacter::StaticClass(), PlayerActors);

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
	
	CurrentAlivePlayers.Append(PlayerActors);
}

void AMobiusGameMode::KillPlayer(AActor* Player)
{
	CurrentAlivePlayers.Remove(Player);
	OnPlayerDiedDelegate.Broadcast(Player);
}
