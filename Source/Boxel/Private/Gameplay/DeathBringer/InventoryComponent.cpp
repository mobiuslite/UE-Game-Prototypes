// 


#include "Gameplay/DeathBringer/InventoryComponent.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/DeathBringer/ResourceDataAsset.h"
#include "Gameplay/Interfaces/InventoryItem.h"
#include "Net/UnrealNetwork.h"

void FResources::AddResource(const FGameplayTag& Tag, const int Count)
{
	bool bFoundItem = false;
	for (int i = 0; i < Resources.Num(); ++i)
	{
		FResourceData& Data = Resources[i];
		if (Data.ResourceTag.MatchesTagExact(Tag))
		{
			Data.ResourceCount += Count;
			MarkItemDirty(Resources[i]);
			bFoundItem = true;
			break;
		}
	}
	
	if (!bFoundItem)
	{
		FResourceData NewData;
		NewData.ResourceTag = Tag;
		NewData.ResourceCount = Count;
		
		Resources.Add(NewData);
		MarkArrayDirty();
	}
}

bool FResources::ConsumeResource(const FGameplayTag& Tag, const int Count)
{
	bool bSuccess = false;
	for (int i = 0; i < Resources.Num(); ++i)
	{
		FResourceData& Data = Resources[i];
		if (Data.ResourceTag.MatchesTagExact(Tag))
		{
			if (Data.ResourceCount > Count)
			{
				Data.ResourceCount -= Count;
				MarkItemDirty(Resources[i]);
				
				bSuccess = true;
			}
			else if (Data.ResourceCount == Count)
			{
				Resources.RemoveAt(i);
				MarkArrayDirty();
				
				bSuccess = true;
			}
			
			break;
		}
	}
	
	return bSuccess;
}

int FResources::GetResourceCount(const FGameplayTag& Tag) const
{
	int Count = 0;
	for (int i = 0; i < Resources.Num(); ++i)
	{
		const FResourceData& Data = Resources[i];
		if (Data.ResourceTag == Tag)
		{
			Count = Data.ResourceCount;
			break;
		}
	}
	
	return  Count;
}

UInventoryComponent::UInventoryComponent()
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	const UClass* Class = UResourceDataAsset::StaticClass();
	AssetRegistryModule.Get().GetAssetsByClass(Class->GetClassPathName(), AssetData, true);
	
	for (int i = 0; i < AssetData.Num(); ++i)
	{
		if (const UResourceDataAsset* ResourceCDO = Cast<UResourceDataAsset>(AssetData[i].GetAsset()))
		{
			AvailableResourceCDOs.Add(ResourceCDO->ResourceTag, ResourceCDO);
		}
	}
	
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ClearInventory();
}

void UInventoryComponent::ClearInventory()
{
	for (int i = 0; i < LargeItems.Num(); ++i)
	{
		RemoveItem(LargeItems[i]);
	}
	
	for (int i = 0; i < SmallItems.Num(); ++i)
	{
		RemoveItem(SmallItems[i]);
	}
	
	Resources.Resources.Empty();
}

void UInventoryComponent::RemoveItem(TScriptInterface<IInventoryItem> Item)
{
	AController* OwnerController = GetOwnerController();
	if (!OwnerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory Component controller is null!"))
	}
	
	Item.GetInterface()->OnUnequip(OwnerController);
	
	LargeItems.Remove(Item);
	
	Item.GetInterface()->OnRemovedFromInventory(this, OwnerController);
}

void UInventoryComponent::AddItem(TScriptInterface<IInventoryItem> Item)
{
	AController* OwnerController = GetOwnerController();
	if (!OwnerController)
	{
		UE_LOG(LogTemp, Error, TEXT("Inventory Component controller is null!"));
		return;
	}
	
	//TODO: Add small item option
	LargeItems.Add(Item);
	
	Item.GetInterface()->OnAddedToInventory(this, OwnerController);
	Item.GetInterface()->OnEquip(OwnerController);
}

bool UInventoryComponent::AddResource(const FGameplayTag& ResourceTag, const int Count)
{
	const UResourceDataAsset* Data = GetResourceCDOFromTag(ResourceTag);
	if (!Data) return false;
	
	int CurrentResourceCount = Resources.GetResourceCount(ResourceTag);

	if (CurrentResourceCount + Count > Data->MaxResourceCount) return false;
	
	Resources.AddResource(ResourceTag, Count);
	CurrentResourceCount += Count;
	
	OnResourceChangedDelegate.Broadcast(ResourceTag, Count, CurrentResourceCount);
	
	return true;
}

bool UInventoryComponent::ConsumeResource(const FGameplayTag& ResourceTag, const int Count)
{
	const bool bSuccess = Resources.ConsumeResource(ResourceTag, Count);

	if (bSuccess)
	{
		OnResourceChangedDelegate.Broadcast(ResourceTag, -Count, Resources.GetResourceCount(ResourceTag));
	}
	
	return bSuccess;
}

int UInventoryComponent::GetResourceCount(const FGameplayTag& ResourceTag) const
{
	return Resources.GetResourceCount(ResourceTag);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ThisClass, LargeItems, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, SmallItems, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, Resources, COND_OwnerOnly);
}

void UInventoryComponent::OnRep_Resources(const FResources& PreviousResources)
{
	FResources CurrentResources = Resources;

	for (int i = 0; i < CurrentResources.Resources.Num(); ++i)
	{
		FResourceData& CurrentData = CurrentResources.Resources[i];
		const int PreviousCount = PreviousResources.GetResourceCount(CurrentData.ResourceTag);

		const int Delta = CurrentData.ResourceCount - PreviousCount;
		if (Delta != 0)
		{
			OnResourceChangedDelegate.Broadcast(CurrentData.ResourceTag, Delta, CurrentData.ResourceCount);
		}
	}
	
}

AController* UInventoryComponent::GetOwnerController() const
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor->IsA(APawn::StaticClass()))
	{
		return Cast<APawn>(OwnerActor)->GetController();
	}
	if (OwnerActor->IsA(APlayerState::StaticClass()))
	{
		const APlayerState* PlayerState = Cast<APlayerState>(OwnerActor);
		return PlayerState->GetOwningController();
	}
	
	return nullptr;
}

const UResourceDataAsset* UInventoryComponent::GetResourceCDOFromTag(const FGameplayTag& Tag)
{
	const UResourceDataAsset* Asset = nullptr;
	if (const UResourceDataAsset** AssetPtr = AvailableResourceCDOs.Find(Tag))
	{
		Asset = *AssetPtr;
	}
	
	return Asset;
}
