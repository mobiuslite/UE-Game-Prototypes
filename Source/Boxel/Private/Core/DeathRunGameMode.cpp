// 


#include "Core/DeathRunGameMode.h"

#include "Gameplay/Deathrun/DeathRunSpawnPoint.h"
#include "Utility/MobiusUtils.h"
#include "GameFramework/PlayerController.h"

void ADeathRunGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<ADeathRunSpawnPoint*> SpawnPoints = UMobiusUtils::GetAllActorsOfClassEX<ADeathRunSpawnPoint>(GetWorld(), ADeathRunSpawnPoint::StaticClass());

	if (SpawnPoints.Num() != 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Please have only 2 death run spawn points!!"));
	}	
	
	for (int i = 0; i < SpawnPoints.Num(); ++i)
	{
		if (SpawnPoints[i]->IsDeathBringerSpawnPoint())
		{
			DeathBringerSpawnPoint = SpawnPoints[i];
		}
		else
		{
			PlayerSpawnPoint = SpawnPoints[i];
		}
	}
	
	if (!PlayerSpawnPoint)
	{
		UE_LOG(LogTemp, Error, TEXT("No player spawn point created!!!"));
	}
	
	if (!DeathBringerSpawnPoint)
	{
		UE_LOG(LogTemp, Error, TEXT("No death bringer spawn point created!!"));
	}
}

void ADeathRunGameMode::StartGame()
{
	Super::StartGame();
	
	if (PreviousDeathBringers.Num() >= CurrentAlivePlayers.Num())
	{
		PreviousDeathBringers.Empty();
	}
	
	TArray<APlayerController*> PossibleDeathBringers;
	for (int i = 0; i < CurrentAlivePlayers.Num(); ++i)
	{
		const APawn* Player = Cast<APawn>(CurrentAlivePlayers[i]);
		if (!Player) continue;
		
		APlayerController* Controller = Player->GetController<APlayerController>();
		if (!Controller) continue;
		
		if (!PreviousDeathBringers.Contains(Controller))
		{
			PossibleDeathBringers.Add(Controller);
		}
	}
	
	if (PossibleDeathBringers.Num() == 0)
	{
		//TODO: Figure out error case
		UE_LOG(LogTemp, Error, TEXT("No possible death bringers!"));
		return;
	}
	
	CurrentDeathBringer = UMobiusUtils::GetRandomItem(PossibleDeathBringers);
	PreviousDeathBringers.Add(CurrentDeathBringer);

	for (int i = 0; i < CurrentAlivePlayers.Num(); ++i)
	{
		APawn* Player = Cast<APawn>(CurrentAlivePlayers[i]);
		if (!Player) continue;
		
		const APlayerController* Controller = Player->GetController<APlayerController>();
		if (!Controller) continue;
		
		const ADeathRunSpawnPoint* SpawnPoint = Controller == CurrentDeathBringer ? DeathBringerSpawnPoint : PlayerSpawnPoint;
		if (!SpawnPoint)
		{
			UE_LOG(LogTemp, Error, TEXT("Tried to teleport a player to a null spawn point"));
			return;
		}
		
		Player->TeleportTo(SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation(), false, true);
	}
}
