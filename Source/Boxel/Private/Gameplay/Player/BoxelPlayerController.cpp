// 


#include "Boxel/Public/Gameplay/Player/BoxelPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Core/DeathBringer/DeathBringerGameMode.h"
#include "Gameplay/DeathBringer/Inventory/DeathBringerItemDataAsset.h"
#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"
#include "Gameplay/Interfaces/InventoryItem.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
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
	
	//Spawn the gun super high in the air, because if it's spawned on a player they pick it up before the PickUpItem below is run.
	//Look into a better way of doing this? Deferred construction won't work because we need all the components to be ready when the player picks it up at all
	FVector ItemSpawnLocation = FVector(100000.0f);
	TScriptInterface<IInventoryItem> NewItem = TScriptInterface<IInventoryItem>(GetWorld()->SpawnActor<AActor>(Data->ItemClass, ItemSpawnLocation, FRotator::ZeroRotator, Params));
	ensure(IsValid(NewItem.GetObject()));
	
	//Items bought from a store need to be destroyed upon round reset, so lets make them transient
	if (ADeathBringerGameMode* GameMode = GetWorld()->GetAuthGameMode<ADeathBringerGameMode>())
	{
		GameMode->AddTransientActor(Cast<AActor>(NewItem.GetObject()));
	}
	
	//If failed to pick up, just place the item at the player
	const bool bSuccess = BoxelCharacter->PickUpItem(NewItem, Data->bConcealOnPurchase);
	if (!bSuccess)
	{
		if (AActor* Actor = Cast<AActor>(NewItem.GetObject()))
		{
			Actor->SetActorLocation(BoxelCharacter->GetActorLocation());
		}
	}
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
