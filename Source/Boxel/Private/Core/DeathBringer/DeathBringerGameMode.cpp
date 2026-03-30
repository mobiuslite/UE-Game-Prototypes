// 


#include "Core/DeathBringer/DeathBringerGameMode.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Core/DeathBringer/DeathBringerGameState.h"
#include "Utility/MobiusUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpectatorPawn.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "Gameplay/Player/BoxelPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"

ADeathBringerGameMode::ADeathBringerGameMode()
{
	GameStateClass = ADeathBringerGameState::StaticClass();
}

void ADeathBringerGameMode::BeginPlay()
{
	Super::BeginPlay();
	SetRoundState(EDeathBringerRoundState::None);
	
	//OnPlayerLogin doesn't get called for the server owner, but we still want them to go through the logic
	if (APlayerController* ServerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		OnPlayerLogin(this, ServerController);
	}
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
	SetRoundState(EDeathBringerRoundState::Preparing);
	GetGameState<ADeathBringerGameState>()->SetRoundEndTime(StartGameWaitTime);
	
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
			
			RestartPlayer(PlayerController);
			
			if (ABoxelPlayerState* PlayerState = PlayerController->GetPlayerState<ABoxelPlayerState>())
			{
				PlayerState->SetGenericTeamId(FGenericTeamId::NoTeam);
			}
			
			if (UMobiusAbilitySystemComponent* AbilityComp = Cast<UMobiusAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerController)))
			{
				if (!AbilityComp->HasMatchingGameplayTag(TAG_Gameplay_Invincible))
				{
					AbilityComp->AddLooseGameplayTag(TAG_Gameplay_Invincible);
				}
				
				AbilityComp->ResetAttributes();
				AbilityComp->ClearAllAbilities();
			}
		}
	}
	
	if (bHardReset)
	{
		//TODO: Clear all guns
	
		//TODO: Spawn in new guns
	}
	
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([this]()
	{
		StartGame();
	}
	), StartGameWaitTime, false);
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
	
	const float DeathBringerRatio = (float)AlivePlayers.Num() * DeathBringerPlayerRatio;
	
	const int NumDeathBringers = FMath::Clamp(FMath::RoundToInt(DeathBringerRatio), MinDeathBringers, 99);
	DeathBringers.Append(UMobiusUtils::GetRandomItems<APawn*>(PossibleDeathBringers, NumDeathBringers));

	for (int i = 0; i < AlivePlayers.Num(); ++i)
	{
		const APawn* Player = AlivePlayers[i];
		if (!Player) continue;
		
		ABoxelPlayerState* PlayerState = Player->GetPlayerState<ABoxelPlayerState>();
		if (!PlayerState) continue;
		
		const int32 TeamId = DeathBringers.Contains(Player) ? DEATHBRINGER_TEAMID : NORMALPLAYER_TEAMID;
		PlayerState->SetGenericTeamId(FGenericTeamId(TeamId));
		
		if (UAbilitySystemComponent* AbilityComp = PlayerState->GetAbilitySystemComponent())
		{
			AbilityComp->RemoveLooseGameplayTag(TAG_Gameplay_Invincible);
		}
	}
	
	SetRoundState(EDeathBringerRoundState::Active);
	GameTimer = GameLengthMinutes * 60.0f;
	GetGameState<ADeathBringerGameState>()->SetRoundEndTime(GameTimer);
}

void ADeathBringerGameMode::EndDeathBringerGame_Implementation(const bool bDeathBringerWin)
{
	SetRoundState(EDeathBringerRoundState::End);
	GetGameState<ADeathBringerGameState>()->SetRoundEndTime(EndGameWaitTime);
	
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([this]()
	{
		PrepareGame(true);
	}
	), EndGameWaitTime, false);
}

void ADeathBringerGameMode::KillPlayer(APawn* Player)
{
	if (!Player) return;

	Super::KillPlayer(Player);
	
	if (DeathBringers.Contains(Player))
	{
		DeathBringers.Remove(Player);
	}
	
	AController* PawnController = Player->GetController();
		
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (ASpectatorPawn* Spectator = GetWorld()->SpawnActor<ASpectatorPawn>(SpectatorClass, Player->GetActorLocation(), Player->GetActorRotation(), Params))
	{
		PawnController->Possess(Spectator);
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

void ADeathBringerGameMode::SetRoundState(const EDeathBringerRoundState RoundState)
{
	GetGameState<ADeathBringerGameState>()->SetRoundState(RoundState);
}

EDeathBringerRoundState ADeathBringerGameMode::GetRoundState() const
{
	return GetGameState<ADeathBringerGameState>()->GetRoundState();
}

bool ADeathBringerGameMode::CanBeDeathBringer(const APlayerController* Controller) const
{
	//TODO: Maybe add a case where someone can't be a death bringer
	return true;
}

void ADeathBringerGameMode::OnPlayerLogin(AGameModeBase* GameMode, APlayerController* PC)
{
	if (GetNumPlayers() >= MinPlayers && GetRoundState() == EDeathBringerRoundState::None)
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