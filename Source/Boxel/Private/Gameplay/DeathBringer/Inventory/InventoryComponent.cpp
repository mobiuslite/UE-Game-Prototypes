// 


#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/DeathBringer/Inventory/ResourceDataAsset.h"
#include "Gameplay/Interfaces/InventoryItem.h"
#include "Gameplay/Player/BoxelPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Utility/AssetRegistryUtils.h"

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

void FResources::ClearResources()
{
	Resources.Empty();
	MarkArrayDirty();
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
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ClearInventory();
}

void UInventoryComponent::ClearInventory()
{
	while (Items.Num() != 0)
	{
		RemoveItem(Items[0]);
	}
	
	Resources.ClearResources();
	
	if (ABoxelPlayerState* PlayerState = Cast<ABoxelPlayerState>(GetOwner()))
	{
		PlayerState->BroadcastGunUnequipped(nullptr);
	}
}

void UInventoryComponent::RemoveItem(AInventoryItem* Item)
{
	AController* OwnerController = GetOwnerController();
	if (!OwnerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory Component controller is null!"))
	}
	
	if (!IsValid(Item)) return;
	
	Items.Remove(Item);
	Item->OnRemovedFromInventory(this, OwnerController);
}

bool UInventoryComponent::AddItem(AInventoryItem* Item)
{
	AController* OwnerController = GetOwnerController();
	if (!OwnerController)
	{
		UE_LOG(LogTemp, Error, TEXT("Inventory Component controller is null!"));
		return false;
	}
	
	if (Items.Contains(Item)) return false;
	
	const EInventoryItem::Type ItemType = Item->GetItemType();
	ensure(ItemType != EInventoryItem::None); // Please set the item type for this actor
	
	const int NumAllowedType = GetMaxItemAmount(ItemType);
	if (Item->GetItemType() != EInventoryItem::None && NumAllowedType > 0)
	{
		int NumItemTypesInInv = 0;
		for (int i = 0; i < Items.Num(); ++i)
		{
			const AInventoryItem* InventoryItem = Items[i];
			if (InventoryItem->GetItemType() == Item->GetItemType())
			{
				NumItemTypesInInv++;
			}
		}
		
		if (NumItemTypesInInv >= NumAllowedType) return false;
	}
	
	Items.Add(Item);
	Item->OnAddedToInventory(this, OwnerController);
	
	return true;
}

bool UInventoryComponent::AddResource(const FGameplayTag& ResourceTag, const int Count)
{
	const UResourceDataAsset* Data = UInventoryComponent::GetResourceCDOFromTag(ResourceTag);
	ensure(Data); //Please add data asset for this resource 
	
	int CurrentResourceCount = Resources.GetResourceCount(ResourceTag);

	if (Data->MaxResourceCount > 0 && CurrentResourceCount + Count > Data->MaxResourceCount) return false;
	
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

AInventoryItem* UInventoryComponent::GetItemByIndex(const int Index) const
{
	if (Index >= Items.Num() || Index < 0) return nullptr;
	
	return Items[Index];
}

int UInventoryComponent::GetIndexOfItem(const AInventoryItem* Item) const
{
	if (!IsValid(Item)) return INDEX_NONE;
	
	return Items.IndexOfByKey(Item);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ThisClass, Items, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, Resources, COND_OwnerOnly);
}

int UInventoryComponent::GetMaxItemAmount(const EInventoryItem::Type& Type)
{
	int Result = 0;
	switch (Type)
	{
	case EInventoryItem::LargeItem:
		{
			Result = MaxLargeItems;
		}
		break;
	case EInventoryItem::SmallItem:
		{
			Result = MaxSmallItems;
		}
		break;
	case EInventoryItem::Utility:
		{
			Result = MaxUtilityItems;
		}
		break;
	}
	
	return Result;
}

void UInventoryComponent::OnRep_Resources(const FResources& PreviousResources)
{
	FResources CurrentResources = Resources;

	for (int i = 0; i < CurrentResources.GetResourcesNum(); ++i)
	{
		FResourceData& CurrentData = CurrentResources.GetResourceAt(i);
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
	
	auto ResourceMap = UAssetRegistryUtils::GetResourceCDOs();
	
	if (const UResourceDataAsset** AssetPtr = ResourceMap.Find(Tag))
	{
		Asset = *AssetPtr;
	}
	
	return Asset;
}
