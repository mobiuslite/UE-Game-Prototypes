// 


#include "Gameplay/Weapons/GunBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Net/UnrealNetwork.h"
#include "Utility/CollisionConsts.h"
#include "GameFramework/Character.h"


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

float AGunBase::GetFireRate() const
{
	return (1.0f / RPM) * 60.0f;
}

void AGunBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentAmmo);
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
	
	const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	if (ACharacter* Character = Cast<ACharacter>(HolderPawn))
	{
		AttachToComponent(Character->GetMesh(), Rules, FName("GunSocket"));
	}
	else
	{
		AttachToActor(Holder, Rules);	
	}
	
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* AbilityComp = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HolderPawn))
		{
			AbilityComp->K2_GiveAbility(GrantedAbilityClass, 0, 1);
		}
	}
}

void AGunBase::RemoveHolder(const APawn* HolderPawn, const bool bThrow)
{
	this->Holder = nullptr;
	SetOwner(nullptr);
	
	SetPhysicsEnabled(true);
	
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
	
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* AbilityComp = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HolderPawn))
		{
			AbilityComp->ClearAllAbilitiesWithInputID(1);
		}
	}
}

