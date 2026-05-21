// 


#include "Gameplay/DeathBringer/Inventory/DroppableInventoryItem.h"

#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/CollisionConsts.h"

ADroppableInventoryItem::ADroppableInventoryItem()
{
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item Mesh"));
	SetRootComponent(ItemMesh);
	
	ItemMesh->SetSimulatePhysics(true);
	ItemMesh->SetCollisionObjectType(ECC_Item);
	
	ItemMesh->SetNotifyRigidBodyCollision(true);
	ItemMesh->SetUseCCD(true);
}

void ADroppableInventoryItem::OnRemovedFromInventory_Implementation(const UInventoryComponent* Inventory,
	AController* HolderController)
{
	UE_LOG(LogTemp, Display, TEXT("Removing holder from item"));
	
	const APawn* HolderPawn = HolderController->GetPawn();
	
	Super::OnRemovedFromInventory_Implementation(Inventory, HolderController);
	
	//TODO: Don't throw on killed death
	if (HolderPawn)
	{
		ItemMesh->AddImpulse(HolderPawn->GetBaseAimRotation().Vector() * DropImpulseStrength, NAME_None, true);
	}
}

void ADroppableInventoryItem::BeginPlay()
{
	Super::BeginPlay();
	ItemMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnItemMeshComponentHit);
}

void ADroppableInventoryItem::SetPhysicsEnabled(const bool bEnabled)
{
	Super::SetPhysicsEnabled(bEnabled);
	ItemMesh->SetSimulatePhysics(bEnabled);
}

void ADroppableInventoryItem::OnHolderHistoryAdded()
{
	ItemMesh->IgnoreActorWhenMoving(GetHolder(), true);
}

void ADroppableInventoryItem::OnHolderHistoryRemoved()
{
	ItemMesh->ClearMoveIgnoreActors();
}

void ADroppableInventoryItem::OnItemMeshComponentHit(UPrimitiveComponent* HitComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	const FHitResult& Hit)
{
	const FVector MeshVelocity = ItemMesh->GetComponentVelocity();
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if (GameState && LastHitWorldTime + MinTimeBetweenHitSound >= GameState->GetServerWorldTimeSeconds())
	{
		return;
	}
	
	const float ImpactDot = Hit.ImpactNormal.Dot(MeshVelocity.GetSafeNormal());
	const float ImpactSpeed = MeshVelocity.Length();
	
	if (FMath::Abs(ImpactDot) > 0.085f && ImpactSpeed > HitSoundSpeedThreshold)
	{
		OnItemHitGround();
		
		if (GameState)
		{
			LastHitWorldTime = GameState->GetServerWorldTimeSeconds();
		}
	}
}

void ADroppableInventoryItem::OnItemHitGround_Implementation()
{
	UGameplayStatics::SpawnSoundAttached(DropSound, ItemMesh, NAME_None, FVector(), FRotator(), EAttachLocation::KeepRelativeOffset, 
			false, 1.0f, 1.0f, 0.0f, DropAttenuationSettings, nullptr, true);
}


