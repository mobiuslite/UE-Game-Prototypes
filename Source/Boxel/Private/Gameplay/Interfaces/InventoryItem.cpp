// 


#include "Gameplay/Interfaces/InventoryItem.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"


AInventoryItem::AInventoryItem()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);
}

bool AInventoryItem::CanBePickedUp(const APawn* PawnHolder) const
{
	bool bResult = true;
	
	for (int i = 0; i < HolderHistory.Num(); ++i)
	{
		const FHolderHistoryData& History = HolderHistory[i];
		if (History.PreviousHolder == PawnHolder)
		{
			bResult = false;
			break;
		}
	}
	
	return bResult;
}

void AInventoryItem::SetPhysicsEnabled(const bool bEnabled)
{
	SetActorEnableCollision(bEnabled);
	SetReplicatingMovement(bEnabled);
}

void AInventoryItem::OnRep_Holder()
{
	SetOwner(HolderPrivate);
	
	//Disable collision while holding
	SetPhysicsEnabled(HolderPrivate == nullptr);
	
	if (HolderPrivate)
	{
		const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
		if (const ACharacter* Character = Cast<ACharacter>(HolderPrivate))
		{
			AttachToComponent(Character->GetMesh(), Rules, FName("GunSocket"));
		}
		else
		{
			AttachToActor(HolderPrivate, Rules);	
		}
	}
	else
	{
		const FDetachmentTransformRules Rules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, false); 
		DetachFromActor(Rules);
		
		//Rotate gun so the side is facing the player that dropped it
		//GunMesh->AddWorldRotation(FRotator(0.0f, 45.0f, 0.0f));
	}
}

void AInventoryItem::OnRep_Visible()
{
	SetActorHiddenInGame(!bVisible);
}

void AInventoryItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, HolderPrivate);
	DOREPLIFETIME(ThisClass, bVisible);
}

void AInventoryItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	for (int i = 0; i < HolderHistory.Num();)
	{
		const FHolderHistoryData& History = HolderHistory[i];
		float Timer = History.HeldCooldownTimer;
		Timer -= DeltaSeconds;
		if (Timer <= 0.0f)
		{
			HolderHistory.RemoveAt(i);
			OnHolderHistoryRemoved();
		}
		else
		{
			FHolderHistoryData DataOverride;
			DataOverride.HeldCooldownTimer = Timer;
			DataOverride.PreviousHolder = History.PreviousHolder;
			HolderHistory[i] = DataOverride;
			i++;
		}
	}
}

// Add default functionality here for any IInventoryItem functions that are not pure virtual.
void AInventoryItem::OnEquip_Implementation(AController* HolderController)
{
	bVisible = true;
	OnRep_Visible();
}

void AInventoryItem::OnUnequip_Implementation(AController* HolderController)
{
	bVisible = false;
	OnRep_Visible();
}

void AInventoryItem::OnAddedToInventory_Implementation(const UInventoryComponent* Inventory,
	AController* HolderController)
{
	HolderPrivate = HolderController->GetPawn();
	OnRep_Holder();
	
	bVisible = false;
	OnRep_Visible();
}

void AInventoryItem::OnRemovedFromInventory_Implementation(const UInventoryComponent* Inventory,
	AController* HolderController)
{
	if (HolderPrivate)
	{
		FHolderHistoryData History;
		History.PreviousHolder = HolderPrivate;
		History.HeldCooldownTimer = 0.5f;
		
		HolderHistory.Add(History);
		OnHolderHistoryAdded();
	}
	
	HolderPrivate = nullptr;
	OnRep_Holder();
	
	bVisible = true;
	OnRep_Visible();
}


