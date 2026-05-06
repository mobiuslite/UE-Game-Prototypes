// 


#include "Core/Lobby/LobbyGameMode.h"

#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
#include "MobiusAbilitySystem/Utils/MAUtils.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Lobby_InArena, "Lobby.InArena");

void ALobbyGameMode::AddPlayerToArena(APlayerController* Controller)
{
	if (ArenaPlayers.Contains(Controller)) return;
	
	const AActor* StartSpot = FindPlayerStart(Controller, "Arena");
	if (!StartSpot) return;
	
	APawn* PlayerPawn = Controller->GetPawn();
	if (!PlayerPawn) return;
	
	if (!PlayerPawn->TeleportTo(StartSpot->GetActorLocation(), StartSpot->GetActorRotation()))
	{
		return;
	}
	
	Controller->SetControlRotation(StartSpot->GetActorRotation());
	
	UMAUtils::RemoveLooseGameplayTagEX(Controller, TAG_Gameplay_Invincible, true);
	UMAUtils::AddLooseGameplayTagEX(Controller, TAG_Lobby_InArena, false, true);
	
	ArenaPlayers.Add(Controller);
}

void ALobbyGameMode::RemovePlayerFromArena(APlayerController* Controller)
{
	if (!ArenaPlayers.Contains(Controller)) return;
	
	UMAUtils::AddLooseGameplayTagEX(Controller, TAG_Gameplay_Invincible, false, true);
	UMAUtils::RemoveLooseGameplayTagEX(Controller, TAG_Lobby_InArena, true);
	
	ArenaPlayers.Remove(Controller);
}

void ALobbyGameMode::KillPlayer(APawn* Player)
{
	if (!Player) return;
	
	Super::KillPlayer(Player);
	
	APlayerController* PawnController = Player->GetController<APlayerController>();
	
	PawnController->ChangeState(NAME_Spectating);
	PawnController->ClientGotoState(NAME_Spectating);
	
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([this, Player, PawnController]()
	{
		if (Player) Player->Destroy();
		
		PawnController->ChangeState(NAME_Playing);
		PawnController->ClientGotoState(NAME_Playing);
		RestartPlayer(PawnController);
	}
	), ArenaRespawnTimer, false);
}

void ALobbyGameMode::OnPlayerLogin(AGameModeBase* GameMode, APlayerController* PC)
{
	UMAUtils::AddLooseGameplayTagEX(PC, TAG_Gameplay_Invincible, false, true);
	UMAUtils::AddLooseGameplayTagEX(PC, TAG_Resources_IgnoreCost, false, true);
}

void ALobbyGameMode::RestartPlayer(AController* NewPlayer)
{
	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
	{
		return;
	}

	//ADDED: Arena player start check
	AActor* StartSpot = FindPlayerStart(NewPlayer, ArenaPlayers.Contains(NewPlayer) ? "Arena" : "Main");

	// If a start spot wasn't found,
	if (StartSpot == nullptr)
	{
		// Check for a previously assigned spot
		if (NewPlayer->StartSpot != nullptr)
		{
			StartSpot = NewPlayer->StartSpot.Get();
			UE_LOG(LogGameMode, Warning, TEXT("RestartPlayer: Player start not found, using last start spot"));
		}	
	}

	RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
}
