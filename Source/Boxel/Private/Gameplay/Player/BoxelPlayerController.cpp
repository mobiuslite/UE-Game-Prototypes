// 


#include "Boxel/Public/Gameplay/Player/BoxelPlayerController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Core/DeathBringer/DeathBringerGameMode.h"
#include "Core/Lobby/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/SpectatorPawn.h"
#include "Gameplay/DeathBringer/StoreItemWorldActor.h"
#include "Gameplay/DeathBringer/Inventory/DeathBringerItemDataAsset.h"
#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"
#include "Gameplay/Interfaces/InventoryItem.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "MobiusAbilitySystem/Utils/MAUtils.h"
#include "Utility/BoxelCheatManager.h"
#include "Utility/MobiusGameplayTags.h"
#include "Utility/MobiusUtils.h"

void ABoxelPlayerController::ShowSpleefModifierName_Implementation(const FString& Name)
{
	BP_ShowSpleefModifierName(Name);
}

void ABoxelPlayerController::RequestItemPurchase_Implementation(const UDeathBringerItemDataAsset* Data)
{
	if (!Data) return;
	
	ABoxelPlayerCharacter* BoxelCharacter = GetPawn<ABoxelPlayerCharacter>();
	if (!BoxelCharacter) return;
	
	if (Data->RequiredTeam != EDeathBringerTeam::None)
	{
		const FGenericTeamId ThisTeamId = UMobiusUtils::GetTeamId(this);
		const EDeathBringerTeam::Type ThisTeam = ADeathBringerGameMode::TeamIDToTeamEnum(ThisTeamId);
		
		if (Data->RequiredTeam != ThisTeam) return;
	}
	
	UInventoryComponent* Inventory;
	if (!UMobiusUtils::GetInventory(GetPlayerState<APlayerState>(), Inventory)) return;
	if (!Inventory->ConsumeResource(TAG_DeathBringer_Currency, Data->Cost)) return;
	
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	if (Data->ItemClass)
	{
		//Spawn the gun super high in the air, because if it's spawned on a player they pick it up before the PickUpItem below is run.
		//Look into a better way of doing this? Deferred construction won't work because we need all the components to be ready when the player picks it up at all
		const FVector ItemSpawnLocation = FVector(100000.0f);
		AInventoryItem* NewItem = GetWorld()->SpawnActor<AInventoryItem>(Data->ItemClass, ItemSpawnLocation, FRotator::ZeroRotator, Params);
		ensure(IsValid(NewItem));
	
		//Items bought from a store need to be destroyed upon round reset, so lets make them transient
		if (ADeathBringerGameMode* GameMode = GetWorld()->GetAuthGameMode<ADeathBringerGameMode>())
		{
			GameMode->AddTransientActor(Cast<AActor>(NewItem));
		}
	
		//If failed to pick up, just place the item at the player
		const bool bSuccess = BoxelCharacter->PickUpItem(NewItem, Data->bConcealOnPurchase);
		if (!bSuccess)
		{
			if (AActor* Actor = Cast<AActor>(NewItem))
			{
				Actor->SetActorLocation(BoxelCharacter->GetActorLocation() + (BoxelCharacter->GetActorForwardVector() * 100.0f));
			}
		}
	}
	
	if (Data->WorldActorClass)
	{
		AStoreItemWorldActor* NewActor = GetWorld()->SpawnActor<AStoreItemWorldActor>(Data->WorldActorClass, 
			BoxelCharacter->GetActorLocation() + (BoxelCharacter->GetActorForwardVector() * 50.0f), FRotator::ZeroRotator, Params);
		NewActor->Initialize(BoxelCharacter);
		
		ensure(IsValid(NewActor));
	}
}

void ABoxelPlayerController::LeaveLobbyArena_Implementation()
{
	if (!UMAUtils::HasLooseGameplayTagEX(this, TAG_Lobby_InArena)) return;
	
	if (ALobbyGameMode* GameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		//If not spectator, kill player
		if (StateName == NAME_Playing)
		{
			UMAUtils::Suicide(this, true);
		}
		
		GameMode->RemovePlayerFromArena(this);
	}
}

ABoxelPlayerController::ABoxelPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CheatClass = UBoxelCheatManager::StaticClass();
}

void ABoxelPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!InputMapping.IsNull())
	{
		if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), 0);
			}
		}
	}
}

void ABoxelPlayerController::BeginSpectatingState()
{
	if (const APawn* LastPawn = GetPawn())
	{
		UnpossessPawnLocation = LastPawn->GetActorLocation();
	}
	
	Super::BeginSpectatingState();
	
	if (ABoxelPlayerState* State = GetPlayerState<ABoxelPlayerState>())
	{
		State->RegisterVoiceChannel(ABoxelPlayerState::DEAD_CHANNELID);
		State->SetIsSpectator(true);
	}
}

void ABoxelPlayerController::EndSpectatingState()
{
	Super::EndSpectatingState();
	
	if (ABoxelPlayerState* State = GetPlayerState<ABoxelPlayerState>())
	{
		State->UnregisterVoiceChannel(ABoxelPlayerState::DEAD_CHANNELID);
		State->SetIsSpectator(false);
		
		PlayerState->SetIsOnlyASpectator(false);
		bPlayerIsWaiting = true;
	}
}

ASpectatorPawn* ABoxelPlayerController::SpawnSpectatorPawn()
{
	ASpectatorPawn* SpawnedSpectator = nullptr;

	// Only spawned for the local player
	if ((GetSpectatorPawn() == nullptr) && IsLocalController())
	{
		UWorld* World = GetWorld();
		if (AGameStateBase const* const GameState = World->GetGameState())
		{
			if (UClass* SpectatorClass = GameState->SpectatorClass)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				SpawnParams.ObjectFlags |= RF_Transient;	// We never want to save spectator pawns into a map
				SpawnedSpectator = World->SpawnActor<ASpectatorPawn>(SpectatorClass, UnpossessPawnLocation, GetControlRotation(), SpawnParams);
				if (SpawnedSpectator)
				{
					SpawnedSpectator->SetReplicates(false); // Client-side only
					SpawnedSpectator->PossessedBy(this);
					SpawnedSpectator->DispatchRestart(true);
					if (SpawnedSpectator->PrimaryActorTick.bStartWithTickEnabled)
					{
						SpawnedSpectator->SetActorTickEnabled(true);
					}

					UE_LOG(LogPlayerController, Verbose, TEXT("Spawned spectator %s [server:%d]"), *GetNameSafe(SpawnedSpectator), GetNetMode() < NM_Client);
				}
				else
				{
					UE_LOG(LogPlayerController, Warning, TEXT("Failed to spawn spectator with class %s"), *GetNameSafe(SpectatorClass));
				}
			}
		}
		else
		{
			// This normally happens on clients if the Player is replicated but the GameState has not yet.
			UE_LOG(LogPlayerController, Verbose, TEXT("NULL GameState when trying to spawn spectator!"));
		}
	}

	return SpawnedSpectator;
}