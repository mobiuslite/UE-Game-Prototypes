// 


#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"

#include "GameFramework/PlayerState.h"
#include "Gameplay/DeathBringer/Inventory/ResourceDataAsset.h"
#include "Gameplay/Interfaces/InventoryItem.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
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
	UE_LOG(LogTemp, Display, TEXT("Clearing Inventory"))

	for (int i = 0; i < Items.Num(); ++i)
	{
		RemoveItem(Items[i], false);
	}
	
	Items.Empty();
	
	const TArray<FResourceData> OldResources = Resources;
	Resources.Empty();
	OnRep_Resources(OldResources);
	
	if (ABoxelPlayerState* PlayerState = Cast<ABoxelPlayerState>(GetOwner()))
	{
		PlayerState->BroadcastGunUnequipped(nullptr);
	}
	
	UE_LOG(LogTemp, Display, TEXT("Done clearing Inventory"))
}

void UInventoryComponent::RemoveItem(AInventoryItem* Item, const bool bAutoRemoveFromList)
{
	AController* OwnerController = GetOwnerController();
	if (!OwnerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory Component controller is null!"))
	}

	if (!IsValid(Item)) return;
	
	//If player is holding this item, set them to be unarmed
	if (ABoxelPlayerCharacter* OwnerCharacter = GetOwnerController()->GetPawn<ABoxelPlayerCharacter>())
	{
		if (OwnerCharacter->GetHeldItem() == Item)
		{
			OwnerCharacter->SetPlayerUnarmed();
		}
	}
	
	if (bAutoRemoveFromList) Items.Remove(Item);
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
	
	TArray<FResourceData> PreviousResources = Resources;
	
	if (Data->MaxResourceCount > 0 && GetResourceCount(ResourceTag) + Count > Data->MaxResourceCount) return false;
	
	bool bFoundItem = false;
	for (int i = 0; i < Resources.Num(); ++i)
	{
		FResourceData& ResourceData = Resources[i];
		if (ResourceData.ResourceTag.MatchesTagExact(ResourceTag))
		{
			ResourceData.ResourceCount += Count;
			bFoundItem = true;
			break;
		}
	}
	
	if (!bFoundItem)
	{
		FResourceData NewData;
		NewData.ResourceTag = ResourceTag;
		NewData.ResourceCount = Count;
		
		Resources.Add(NewData);
	}
	
	OnRep_Resources(PreviousResources);
	
	return true;
}

bool UInventoryComponent::ConsumeResource(const FGameplayTag& ResourceTag, const int Count)
{
	TArray<FResourceData> PreviousResources = Resources;
	
	bool bSuccess = false;
	for (int i = 0; i < Resources.Num(); ++i)
	{
		FResourceData& Data = Resources[i];
		if (Data.ResourceTag.MatchesTagExact(ResourceTag))
		{
			if (Data.ResourceCount > Count)
			{
				Data.ResourceCount -= Count;
				bSuccess = true;
			}
			else if (Data.ResourceCount == Count)
			{
				Resources.RemoveAt(i);
				bSuccess = true;
			}
			
			break;
		}
	}

	if (bSuccess)
	{
		OnRep_Resources(PreviousResources);
	}
	
	return bSuccess;
}

int UInventoryComponent::GetResourceCount(const FGameplayTag& ResourceTag) const
{
	int Count = 0;

	for (int i = 0; i < Resources.Num(); ++i)
	{
		if (Resources[i].ResourceTag == ResourceTag)
		{
			Count = Resources[i].ResourceCount;
			break;
		}
	}
	
	return Count;
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

TArray<FString> UInventoryComponent::GetItemNames() const
{
	TArray<FString> Results;
	for (int i = 0; i < Items.Num(); ++i)
	{
		Results.Add(Items[i]->GetName());
	}
	return Results;
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

void UInventoryComponent::OnRep_Resources(const TArray<FResourceData>& PreviousResources)
{
	TMap<FGameplayTag, int> ResourceDelta;
	
	//Assume all resources are being removed until they appear in the current resources
	for (int i = 0; i < PreviousResources.Num(); i++)
	{
		const FResourceData& PreviousData = PreviousResources[i];
		ResourceDelta.Add(PreviousData.ResourceTag, -PreviousData.ResourceCount);
	}
	
	for (int i = 0; i < Resources.Num(); ++i)
	{
		FResourceData& CurrentData = Resources[i];

		const int PreviousAmount = ResourceDelta.Contains(CurrentData.ResourceTag) ? ResourceDelta[CurrentData.ResourceTag] : 0;
		
		const int Delta = CurrentData.ResourceCount + PreviousAmount;
		ResourceDelta.Remove(CurrentData.ResourceTag);
		
		if (Delta != 0)
		{
			OnResourceChangedDelegate.Broadcast(CurrentData.ResourceTag, Delta, CurrentData.ResourceCount);
		}
	}

	for (auto Delta : ResourceDelta)
	{
		OnResourceChangedDelegate.Broadcast(Delta.Key, Delta.Value, 0);
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
