#include "GameplayDebuggerCategory_Boxel.h"

#include "Gameplay/Player/BoxelPlayerState.h"
#include "Utility/MobiusUtils.h"

FGameplayDebuggerCategory_Boxel::FGameplayDebuggerCategory_Boxel()
{
	SetDataPackReplication(&DataPack);
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_Boxel::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_Boxel());
}

void FGameplayDebuggerCategory_Boxel::FRepData::Serialize(FArchive& Ar)
{
	Ar << TeamId;
	Ar << RegisteredVCs;
	Ar << InventoryItems;
	Ar << InventoryResources;
}

void FGameplayDebuggerCategory_Boxel::DrawData(APlayerController* OwnerPC,
                                               FGameplayDebuggerCanvasContext& CanvasContext)
{
	CanvasContext.Printf(TEXT("{white}Team ID: {yellow}%d"), DataPack.TeamId);

	if (DataPack.RegisteredVCs.Num() > 0)
	{
		CanvasContext.Printf(TEXT("{white}Registered Voice Channels"));
	}	
	for (int i = 0; i < DataPack.RegisteredVCs.Num(); ++i)
	{
		CanvasContext.Printf(TEXT("{yellow}Channel ID:%d"), DataPack.RegisteredVCs[i]);
	}
	
	CanvasContext.Printf(TEXT("{white}Inventory:"));
	
	if (DataPack.InventoryItems.Num() > 0)
	{
		CanvasContext.Printf(TEXT("{white}Items"));
	}
	for (int i = 0; i < DataPack.InventoryItems.Num(); ++i)
	{
		if (!DataPack.InventoryItems[i].IsEmpty())
		{
			CanvasContext.Printf(TEXT("{yellow}%s"), *DataPack.InventoryItems[i]);
		}
	}
	
	if (DataPack.InventoryResources.Num() > 0)
	{
		CanvasContext.Printf(TEXT("{white}Resources"));
	}
	for (int i = 0; i < DataPack.InventoryResources.Num(); ++i)
	{
		CanvasContext.Printf(TEXT("{yellow}%s:%d"), *DataPack.InventoryResources[i].ResourceTag.ToString(), DataPack.InventoryResources[i].ResourceCount);
	}
}

void FGameplayDebuggerCategory_Boxel::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	if (!OwnerPC || !DebugActor) return;

	if (const APawn* Pawn = Cast<APawn>(DebugActor))
	{
		if (const ABoxelPlayerState* PlayerState = Pawn->GetPlayerState<ABoxelPlayerState>())
		{
			DataPack.TeamId = PlayerState->GetTeamId().GetId();
			DataPack.RegisteredVCs = PlayerState->GetRegisteredVoiceChannels();
			
			UInventoryComponent* Inventory;
			if (UMobiusUtils::GetInventory(PlayerState, Inventory))
			{
				DataPack.InventoryItems = Inventory->GetItemNames();
				DataPack.InventoryResources = Inventory->GetResources();
			}
		}
	}
}
