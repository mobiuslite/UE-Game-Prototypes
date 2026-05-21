// 


#include "Core/Lobby/LobbyGameMode.h"

#include "AbilitySystemGlobals.h"
#include "Core/Lobby/LobbyGameState.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/PlayerStartPIE.h"
#include "GameFramework/PlayerStart.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "Gameplay/Weapons/GunBase.h"
#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
#include "MobiusAbilitySystem/Utils/MAUtils.h"
#include "Utility/MobiusUtils.h"
#include "GameFramework/PlayerState.h"
#include "MobiusAbilitySystem/Utils/MAGameplayTags.h"

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
	
	GivePlayerGun(PlayerPawn);
}

void ALobbyGameMode::RemovePlayerFromArena(APlayerController* Controller)
{
	if (!ArenaPlayers.Contains(Controller)) return;
	
	UMAUtils::AddLooseGameplayTagEX(Controller, TAG_Gameplay_Invincible, false, true);
	UMAUtils::RemoveLooseGameplayTagEX(Controller, TAG_Lobby_InArena, true);
	
	ArenaPlayers.Remove(Controller);
}

void ALobbyGameMode::KillPlayer(APawn* Player, const AController* KilledBy)
{
	if (!Player) return;
	
	Super::KillPlayer(Player, KilledBy);
	
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
	
	if (ALobbyGameState* LobbyGameState = GetGameState<ALobbyGameState>())
	{
		const APlayerState* KilledPlayerState = PawnController->GetPlayerState<APlayerState>();
		LobbyGameState->AddKillHistory(KilledBy ? KilledBy->GetPlayerState<APlayerState>() : KilledPlayerState, KilledPlayerState);
	}
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

	const bool bInArena = ArenaPlayers.Contains(NewPlayer);
	
	//ADDED: Arena player start check
	AActor* StartSpot = FindPlayerStart(NewPlayer, bInArena ? "Arena" : "Main");

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
	
	if (bInArena)
	{
		GivePlayerGun(NewPlayer->GetPawn());
		
		if (UMobiusAbilitySystemComponent* ASC = Cast<UMobiusAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(NewPlayer)))
		{
			ASC->ResetAttributes();
		}
	}
}

AActor* ALobbyGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	UWorld* World = GetWorld();

	// If incoming start is specified, then just use it
	if (!IncomingName.IsEmpty())
	{
		TArray<APlayerStart*> PossiblePlayerStarts;
		//ADDED: Selects a random player start from the incoming name if more than one exists
		//Normally in unreal it just returns the first instance
		const FName IncomingPlayerStartTag = FName(*IncomingName);
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* Start = *It;
			if (Start && Start->PlayerStartTag == IncomingPlayerStartTag)
			{
				PossiblePlayerStarts.Add(Start);
			}
			
			if (Start->IsA<APlayerStartPIE>() && IncomingName == "Main")
			{
				// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
				return Start;
			}
		}
		
		return UMobiusUtils::GetRandomItem(PossiblePlayerStarts);
	}

	// Always pick StartSpot at start of match
	if (ShouldSpawnAtStartSpot(Player))
	{
		if (AActor* PlayerStartSpot = Player->StartSpot.Get())
		{
			return PlayerStartSpot;
		}
		else
		{
			UE_LOG(LogGameMode, Error, TEXT("FindPlayerStart: ShouldSpawnAtStartSpot returned true but the Player StartSpot was null."));
		}
	}

	AActor* BestStart = ChoosePlayerStart(Player);
	if (BestStart == nullptr)
	{
		// No player start found
		UE_LOG(LogGameMode, Log, TEXT("FindPlayerStart: PATHS NOT DEFINED or NO PLAYERSTART with positive rating"));

		// This is a bit odd, but there was a complex chunk of code that in the end always resulted in this, so we may as well just 
		// short cut it down to this.  Basically we are saying spawn at 0,0,0 if we didn't find a proper player start
		BestStart = World->GetWorldSettings();
	}

	return BestStart;
}

void ALobbyGameMode::GivePlayerGun(APawn* Pawn)
{
	ABoxelPlayerCharacter* BoxelPlayer = Cast<ABoxelPlayerCharacter>(Pawn);
	if (!BoxelPlayer) return;
	
	TArray<UClass*> GunCDOs;
	ArenaGuns->GetObjects(GunCDOs);
	if (GunCDOs.Num() == 0) return;
	
	UClass* GunClass = UMobiusUtils::GetRandomItem(GunCDOs);
		
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
			
	AGunBase* NewGun = GetWorld()->SpawnActor<AGunBase>(GunClass, SpawnParams);
	NewGun->SetCanBeDropped(false);
			
	BoxelPlayer->PickUpItem(NewGun);
}
