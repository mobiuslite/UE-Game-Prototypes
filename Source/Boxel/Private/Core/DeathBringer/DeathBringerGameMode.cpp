// 


#include "Core/DeathBringer/DeathBringerGameMode.h"

#include "Core/DeathBringer/DeathBringerGameState.h"
#include "Utility/MobiusUtils.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "Gameplay/Player/BoxelPlayerState.h"
#include "MobiusAbilitySystem/Utils/MAGameplayTags.h"
#include "MobiusAbilitySystem/Utils/MAUtils.h"
#include "Utility/MobiusGameplayTags.h"

ADeathBringerGameMode::ADeathBringerGameMode()
{
	GameStateClass = ADeathBringerGameState::StaticClass();
}

TEnumAsByte<EDeathBringerTeam::Type> ADeathBringerGameMode::TeamIDToTeamEnum(const struct FGenericTeamId& TeamId)
{
	if (TeamId.GetId() == NOTEAM_TEAMID) return EDeathBringerTeam::Type::None;
	
	return (EDeathBringerTeam::Type)TeamId.GetId();
}

void ADeathBringerGameMode::BeginPlay()
{
	Super::BeginPlay();
	SetRoundState(EDeathBringerRoundState::None);
}

void ADeathBringerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

void ADeathBringerGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetRoundState() == EDeathBringerRoundState::Active)
	{
		GameTimer -= DeltaSeconds;
		if (GameTimer <= 0.0f)
		{
			EndDeathBringerGame(false);
		}
	}
}

AActor* ADeathBringerGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	UWorld* World = GetWorld();
	
	//REMOVED: Removed default starting spots and made it always pick a new starting spot

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

void ADeathBringerGameMode::PrepareGame(const bool bHardReset)
{
	UE_LOG(LogTemp, Display, TEXT("Preparing game"))
	
	const FDeathBringerGameModeSettings& GameSettings = GetGameModeSettings();
	
	SetRoundState(EDeathBringerRoundState::Preparing);
	GetGameState<ADeathBringerGameState>()->SetRoundEndTime(GameSettings.StartGameWaitTime);

	if (bHardReset)
	{
		UE_LOG(LogTemp, Display, TEXT("Hard resetting game"))
		
		if (AMobiusGameState* State = GetGameState<AMobiusGameState>())
		{
			State->BroadcastRoundReset();
		}
		
		//Cleans up all remaining players and dead bodies where no controller is posessing them
		const TArray<ABoxelPlayerCharacter*> PlayerPawns = UMobiusUtils::GetAllActorsOfClassEX<ABoxelPlayerCharacter>(GetWorld(), ABoxelPlayerCharacter::StaticClass());
		for (int i = 0; i < PlayerPawns.Num(); ++i)
		{
			if (ABoxelPlayerCharacter* BoxelCharacter = PlayerPawns[i])
			{
				BoxelCharacter->Destroy();
			}
		}
	}
	
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerState)
		{
			if (bHardReset)
			{
				if (APawn* CurrentPawn = PlayerController->GetPawnOrSpectator())
				{
					CurrentPawn->Destroy();
				}
			}
			
			PlayerController->ChangeState(NAME_Playing);
			PlayerController->ClientGotoState(NAME_Playing);
			RestartPlayer(PlayerController);
			
			UMAUtils::AddLooseGameplayTagEX(PlayerController, TAG_Gameplay_Invincible, false, false);
		}
	}
	
	if (bHardReset)
	{
		for (int i = 0; i < TransientActors.Num(); ++i)
		{
			if (AActor* Actor = TransientActors[i]) Actor->Destroy();
		}
		TransientActors.Empty();
	}
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
	{
		StartGame();
	}
	), GameSettings.StartGameWaitTime, false);
	
	UE_LOG(LogTemp, Display, TEXT("Done Preparing game"))
}

void ADeathBringerGameMode::StartGame()
{
	UE_LOG(LogTemp, Display, TEXT("Starting game"))
	
	Super::StartGame();
	DeathBringers.Empty();
	
	TArray<APawn*> PossibleDeathBringers;
	for (int i = 0; i < AlivePlayers.Num(); ++i)
	{
		APawn* Player = AlivePlayers[i];
		if (!Player) continue;
		
		APlayerController* Controller = Player->GetController<APlayerController>();
		if (!Controller) continue;
		
		if (CanBeDeathBringer(Controller))
		{
			PossibleDeathBringers.Add(Player);
		}
	}
	
	if (PossibleDeathBringers.Num() == 0)
	{
		//TODO: Figure out error case
		UE_LOG(LogTemp, Error, TEXT("No possible death bringers!"));
		return;
	}
	
	const FDeathBringerGameModeSettings& GameSettings = GetGameModeSettings();
	
	const float DeathBringerRatio = (float)AlivePlayers.Num() * GameSettings.DeathBringerPlayerRatio;
	
	const int NumDeathBringers = FMath::Clamp(FMath::RoundToInt(DeathBringerRatio), GameSettings.MinDeathBringers, 99);
	DeathBringers.Append(UMobiusUtils::GetRandomItems<APawn*>(PossibleDeathBringers, NumDeathBringers));
	
	int RandomSaviourIndex = -1;
	if (DeathBringers.Num() >= GameSettings.MinDeathBringersForSaviour)
	{
		TArray<int> ValidIndices;
		for (int i = 0; i < AlivePlayers.Num(); ++i)
		{
			if (DeathBringers.Contains(AlivePlayers[i]))
			{
				continue;
			}
			
			ValidIndices.Add(i);
		}
		RandomSaviourIndex = UMobiusUtils::GetRandomItem(ValidIndices);
	}
	
	for (int i = 0; i < AlivePlayers.Num(); ++i)
	{
		APawn* Player = AlivePlayers[i];
		if (!Player) continue;
		
		ABoxelPlayerState* PlayerState = Player->GetPlayerState<ABoxelPlayerState>();
		if (!PlayerState) continue;
		
		PlayerState->UnregisterUncommonVoiceChannels();
		
		int32 TeamId = DeathBringers.Contains(Player) ? EDeathBringerTeam::DeathBringer : EDeathBringerTeam::Normal;
		if (i == RandomSaviourIndex) TeamId = EDeathBringerTeam::Saviour;
		
		PlayerState->SetGenericTeamId(FGenericTeamId(TeamId));
		
		if (HasTeamVoiceChannel(TeamId))
		{
			PlayerState->RegisterVoiceChannel(TeamId);
		}
		
		UMAUtils::RemoveLooseGameplayTagEX(PlayerState, TAG_Gameplay_Invincible, false);
		UMAUtils::RemoveLooseGameplayTagEX(PlayerState, TAG_DeathBringer_Guilty, true);
		
		bool bIsGuilty = false;
		AController* Controller = Player->GetController();
		if (TeamKillers.Contains(Controller))
		{
			const int TeamKillCount = TeamKillers[Controller];
			bIsGuilty = true;
			
			UMAUtils::AddLooseGameplayTagEX(Controller, TAG_DeathBringer_Guilty, true , true, TeamKillCount);
			TeamKillers.Remove(Controller);
			
			if (ABoxelPlayerCharacter* Character = Cast<ABoxelPlayerCharacter>(Player))
			{
				TArray<uint8> Payload;
				Payload.Add(UMAUtils::GetLooseGameplayTagCountEX(Controller, TAG_DeathBringer_Guilty));
				Character->Client_SendGenericMessage(EGenericPlayerMessage::Guilty, Payload);
			}
		}
		else if (UMAUtils::HasLooseGameplayTagEX(Controller, TAG_DeathBringer_Guilty))
		{
			bIsGuilty = true;
			
			if (ABoxelPlayerCharacter* Character = Cast<ABoxelPlayerCharacter>(Player))
			{
				TArray<uint8> Payload;
				Payload.Add(UMAUtils::GetLooseGameplayTagCountEX(Controller, TAG_DeathBringer_Guilty));
				Character->Client_SendGenericMessage(EGenericPlayerMessage::Guilty, Payload);
			}
		}
		
		//Special roles get sum money to spend, yippee!!
		if (TeamId != EDeathBringerTeam::Normal && !bIsGuilty)
		{
			UInventoryComponent* Inventory;
			if (UMobiusUtils::GetInventory(PlayerState, Inventory))
			{
				Inventory->AddResource(TAG_DeathBringer_Currency, GameSettings.CurrencyStartAmount);
			}
		}
	}

	if (DeathBringers.Num() > 1)
	{
		for (int i = 0; i < DeathBringers.Num(); ++i)
		{
			ABoxelPlayerState* CurrentPlayerState = DeathBringers[i]->GetPlayerState<ABoxelPlayerState>();
			if (!CurrentPlayerState) continue;

			TArray<ABoxelPlayerState*> TeamPlayerStates;
			
			for (int j = 0; j < DeathBringers.Num(); ++j)
			{
				if (i == j) continue;

				if (ABoxelPlayerState* OtherPlayerState = DeathBringers[j]->GetPlayerState<ABoxelPlayerState>())
				{
					TeamPlayerStates.Add(OtherPlayerState);
				}
			}
		
			CurrentPlayerState->SetTeammates(FGenericTeamId(EDeathBringerTeam::DeathBringer), TeamPlayerStates);
		}
	}
	
	TeamKillers.Empty();
	
	SetRoundState(EDeathBringerRoundState::Active);
	GameTimer = GameSettings.GameLengthMinutes * 60.0f;
	GetGameState<ADeathBringerGameState>()->SetRoundEndTime(GameTimer);
	
	UE_LOG(LogTemp, Display, TEXT("Finished Starting game"))
}

void ADeathBringerGameMode::EndDeathBringerGame_Implementation(const bool bDeathBringerWin)
{
	const FDeathBringerGameModeSettings& GameSettings = GetGameModeSettings();
	
	if (ADeathBringerGameState* State = GetGameState<ADeathBringerGameState>())
	{
		if (State->GetRoundState() == EDeathBringerRoundState::End) return;
		
		State->SetRoundState(EDeathBringerRoundState::End);
		State->SetRoundEndTime(GameSettings.EndGameWaitTime);
		State->ShowRoundEndToast(bDeathBringerWin);
	} 
	
	TArray<ABoxelPlayerState*> AllPlayerStates = UMobiusUtils::GetAllActorsOfClassEX<ABoxelPlayerState>(GetWorld(), ABoxelPlayerState::StaticClass());
	for (int i = 0; i < AllPlayerStates.Num(); i++)
	{
		if (AllPlayerStates[i])
		{
			AllPlayerStates[i]->RegisterVoiceChannel(ABoxelPlayerState::DEAD_CHANNELID);
		}
	}
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
	{
		PrepareGame(true);
	}
	), GameSettings.EndGameWaitTime, false);
}

void ADeathBringerGameMode::KillPlayer(APawn* Player, const AController* KilledBy)
{
	if (!Player) return;

	Super::KillPlayer(Player, KilledBy);
	
	if (DeathBringers.Contains(Player))
	{
		DeathBringers.Remove(Player);
	}
	
	APlayerController* PawnController = Player->GetController<APlayerController>();
	
	PawnController->ChangeState(NAME_Spectating);
	PawnController->ClientGotoState(NAME_Spectating);
	
	if (GetRoundState() == EDeathBringerRoundState::Active)
	{
		if (UMobiusUtils::GetTeamAttitude(UMobiusUtils::GetTeamId(KilledBy),  UMobiusUtils::GetTeamId(PawnController)) == ETeamAttitude::Friendly)
		{
			int& KillCount = TeamKillers.FindOrAdd(KilledBy);
			KillCount++;
		}
	}
	
	if (AlivePlayers.Num() == DeathBringers.Num())
	{
		EndDeathBringerGame(true);
	}
	else if (DeathBringers.Num() == 0)
	{
		EndDeathBringerGame(false);
	}
}

void ADeathBringerGameMode::AddTransientActor(AActor* Actor)
{
	TransientActors.Add(Actor);
}

void ADeathBringerGameMode::SetRoundState(const EDeathBringerRoundState::Type RoundState)
{
	GetGameState<ADeathBringerGameState>()->SetRoundState(RoundState);
}

EDeathBringerRoundState::Type ADeathBringerGameMode::GetRoundState() const
{
	return GetGameState<ADeathBringerGameState>()->GetRoundState();
}

const FDeathBringerGameModeSettings& ADeathBringerGameMode::GetGameModeSettings() const
{
#if !WITH_EDITOR
	return ShippingGameModeSettings;
#else
	return bUseDebugSettings ? DebugGameModeSettings : ShippingGameModeSettings;
#endif
}

bool ADeathBringerGameMode::CanBeDeathBringer(const APlayerController* Controller) const
{
	//TODO: Maybe add a case where someone can't be a death bringer
	return true;
}

bool ADeathBringerGameMode::HasTeamVoiceChannel(const int32 TeamID)
{
	return TeamID == EDeathBringerTeam::DeathBringer;
}

void ADeathBringerGameMode::OnPlayerLogin(AGameModeBase* GameMode, APlayerController* PC)
{
	if (GetNumPlayers() >= GetGameModeSettings().MinPlayers && GetRoundState() == EDeathBringerRoundState::None)
	{
		PrepareGame(false);
	}
	
	UMAUtils::AddLooseGameplayTagEX(PC, TAG_Gameplay_Invincible, false, false);
}

void ADeathBringerGameMode::OnPlayerLogout(AGameModeBase* GameMode, AController* PC)
{
	Super::OnPlayerLogout(GameMode, PC);
	KillPlayer(PC->GetPawn(), nullptr);
}