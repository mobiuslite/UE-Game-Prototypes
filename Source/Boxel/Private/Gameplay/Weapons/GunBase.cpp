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
	this->Holder = HolderPawn;
	SetOwner(HolderPawn);
	
	//Disable collision while holding
	SetActorEnableCollision(HolderPawn == nullptr);
	
	bTriggerDown = false;
	
	const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	AttachToActor(Holder, Rules, FName("GunSocket"));
}

