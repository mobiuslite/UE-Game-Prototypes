// 


#include "Core/DeathBringerGameMode.h"

#include "Utility/MobiusUtils.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/Player/BoxelPlayerState.h"

void ADeathBringerGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ADeathBringerGameMode::StartGame()
{
	Super::StartGame();
	DeathBringers.Empty();
	
	TArray<APlayerController*> PossibleDeathBringers;
	for (int i = 0; i < AlivePlayers.Num(); ++i)
	{
		const APawn* Player = Cast<APawn>(AlivePlayers[i]);
		if (!Player) continue;
		
		APlayerController* Controller = Player->GetController<APlayerController>();
		if (!Controller) continue;
		
		if (!CanBeDeathBringer(Controller))
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
	
	const float DeathBringerRatio = (float)AlivePlayers.Num() * DeathBringerPlayerRatio;
	
	const int NumDeathBringers = FMath::Clamp(FMath::RoundToInt(DeathBringerRatio), MinDeathBringers, 99);
	DeathBringers.Append(UMobiusUtils::GetRandomItems<APlayerController*>(PossibleDeathBringers, NumDeathBringers));

	for (int i = 0; i < AlivePlayers.Num(); ++i)
	{
		const APawn* Player = Cast<APawn>(AlivePlayers[i]);
		if (!Player) continue;
		
		ABoxelPlayerState* PlayerState = Player->GetPlayerState<ABoxelPlayerState>();
		if (!PlayerState) continue;
		
		const int32 TeamId = DeathBringers.Contains(Player->GetController()) ? DEATHBRINGER_TEAMID : NORMALPLAYER_TEAMID;
		PlayerState->SetGenericTeamId(FGenericTeamId(TeamId));
	}
}

bool ADeathBringerGameMode::CanBeDeathBringer(const APlayerController* Controller) const
{
	//TODO: Maybe add a case where someone can't be a death bringer
	return true;
}
