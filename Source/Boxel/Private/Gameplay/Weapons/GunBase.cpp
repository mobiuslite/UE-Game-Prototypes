// 


#include "Gameplay/Weapons/GunBase.h"

#include "Utility/CollisionConsts.h"
#include "Utility/MobiusUtils.h"


AGunBase::AGunBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);
	
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh"));
	SetRootComponent(GunMesh);
	
	GunMesh->SetCollisionObjectType(ECC_Gun);
	GunMesh->SetSimulatePhysics(true);
	
}

void AGunBase::PullTrigger()
{
	bTriggerDown = true;
}

void AGunBase::ReleaseTrigger()
{
	bTriggerDown = false;
}

void AGunBase::BeginPlay()
{
	Super::BeginPlay();
}

void AGunBase::SetPhysicsEnabled(const bool bEnabled)
{
	SetActorEnableCollision(bEnabled);
	GunMesh->SetSimulatePhysics(bEnabled);
	SetReplicatingMovement(bEnabled);
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (FireTimer > 0.0f)
	{
		FireTimer -= DeltaTime;
	}
	
	if (Holder && bTriggerDown && FireTimer <= 0.0f)
	{
		const float SecondsBetweenFire = (1.0f / RPM) * 60.0f;
		FireTimer += SecondsBetweenFire;
		
		FVector CameraLocation;
		UMobiusUtils::GetCameraLocation(Holder->GetController(), CameraLocation);
	
		CameraLocation += Holder->GetControlRotation().Vector();
		
		Server_FireProjectile(CameraLocation, Holder->GetControlRotation());
	
		if (!HasAuthority())
		{
			FireProjectile(CameraLocation, Holder->GetControlRotation());
		}
	}
}

void AGunBase::Server_FireProjectile_Implementation(const FVector& Location, const FRotator& Rotation)
{
	FireProjectile(Location, Rotation);
}

void AGunBase::SetHolder(APawn* HolderPawn)
{
	if (!HolderPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("GunBase: NO HOLDER GIVEN. Please pass a valid holder. If you're trying to remove the holder, use RemoveHolder"));
		return;
	}
	
	this->Holder = HolderPawn;
	SetOwner(HolderPawn);
	
	//Disable collision while holding
	SetPhysicsEnabled(false);
	
	bTriggerDown = false;
	
	const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	AttachToActor(Holder, Rules);
		
	SetActorRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	SetActorRelativeLocation(FVector(20.0f, 30.0f, 34.0f));
}

void AGunBase::RemoveHolder(const APawn* HolderPawn, const bool bThrow)
{
	this->Holder = nullptr;
	SetOwner(nullptr);
	
	SetPhysicsEnabled(true);
	
	bTriggerDown = false;
	
	if (bThrow)
	{
		AddActorWorldOffset(HolderPawn->GetBaseAimRotation().Vector() * DropThrowOffset);
	}
	
	const FDetachmentTransformRules Rules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, false); 
	DetachFromActor(Rules);
	
	if (bThrow)
	{
		//Rotate gun so the side is facing the player that dropped it
		GunMesh->AddWorldRotation(FRotator(0.0f, 45.0f, 0.0f));
		
		if (HasAuthority())
		{
			GunMesh->AddImpulse(HolderPawn->GetBaseAimRotation().Vector() * DropImpulseStrength, NAME_None, true);
		}
	}
}

