#pragma once

#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"
#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"

class APlayerController;
class AActor;

class FGameplayDebuggerCategory_Boxel : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_Boxel();
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;
    
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();
    
protected:
	struct FRepData
	{
		int TeamId;
		TArray<uint8> RegisteredVCs;
		
		TArray<FResourceData> InventoryResources;
		TArray<FString> InventoryItems;
        
		void Serialize(FArchive& Ar);
	};
    
	FRepData DataPack;
};

#endif
