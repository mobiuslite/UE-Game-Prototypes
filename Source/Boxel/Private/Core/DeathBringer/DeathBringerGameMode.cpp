// 


#include "Core/DeathBringer/DeathBringerGameMode.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Core/DeathBringer/DeathBringerGameState.h"
#include "Utility/MobiusUtils.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "Gameplay/Player/BoxelPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
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

void ADeathBringerGameMode::PrepareGame(const bool bHardReset)
{
	const FDeathBringerGameModeSettings& GameSettings = GetGameModeSettings();
	
	SetRoundState(EDeathBringerRoundState::Preparing);
	GetGameState<ADeathBringerGameState>()->SetRoundEndTime(GameSettings.StartGameWaitTime);

	if (bHardReset)
	{
		//Cleans up all remaining players and dead bodies where no controller is posessing them
		const TArray<ABoxelPlayerCharacter*> PlayerPawns = UMobiusUtils::GetAllActorsOfClassEX<ABoxelPlayerCharacter>(GetWorld(), ABoxelPlayerCharacter::StaticClass());
		for (int i = 0; i < PlayerPawns.Num(); ++i)
		{
			if (ABoxelPlayerCharacter* BoxelCharacter = PlayerPawns[i])
			{
				BoxelCharacter->Destroy();
			}
		}
		
		if (AMobiusGameState* State = GetGameState<AMobiusGameState>())
		{
			State->BroadcastRoundReset();
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
			
			if (UMobiusAbilitySystemComponent* AbilityComp = Cast<UMobiusAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerController->PlayerState)))
			{
				if (!AbilityComp->HasMatchingGameplayTag(TAG_Gameplay_Invincible))
				{
					AbilityComp->AddLooseGameplayTag(TAG_Gameplay_Invincible);
				}
			}
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
	
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([this]()
	{
		StartGame();
	}
	), GameSettings.StartGameWaitTime, false);
}

void ADeathBringerGameMode::StartGame()
{
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
		const APawn* Player = AlivePlayers[i];
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
		
		if (UAbilitySystemComponent* AbilityComp = PlayerState->GetAbilitySystemComponent())
		{
			AbilityComp->RemoveLooseGameplayTag(TAG_Gameplay_Invincible);
		}
		
		//Not normal player get sum money to spend, yippee!!
		if (TeamId != EDeathBringerTeam::Normal)
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
	
	SetRoundState(EDeathBringerRoundState::Active);
	GameTimer = GameSettings.GameLengthMinutes * 60.0f;
	GetGameState<ADeathBringerGameState>()->SetRoundEndTime(GameTimer);
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
	
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([this]()
	{
		PrepareGame(true);
	}
	), GameSettings.EndGameWaitTime, false);
}

void ADeathBringerGameMode::KillPlayer(APawn* Player)
{
	if (!Player) return;

	Super::KillPlayer(Player);
	
	if (DeathBringers.Contains(Player))
	{
		DeathBringers.Remove(Player);
	}
	
	APlayerController* PawnController = Player->GetController<APlayerController>();
	
	PawnController->ChangeState(NAME_Spectating);
	PawnController->ClientGotoState(NAME_Spectating);
	
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
#endif
	
	return bUseDebugSettings ? DebugGameModeSettings : ShippingGameModeSettings;
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
	
	if (UAbilitySystemComponent* AbilityComp = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PC))
	{
		if (!AbilityComp->HasMatchingGameplayTag(TAG_Gameplay_Invincible))
		{
			AbilityComp->AddLooseGameplayTag(TAG_Gameplay_Invincible);
		}
	}
}

void ADeathBringerGameMode::OnPlayerLogout(AGameModeBase* GameMode, AController* PC)
{
	Super::OnPlayerLogout(GameMode, PC);
	KillPlayer(PC->GetPawn());
}