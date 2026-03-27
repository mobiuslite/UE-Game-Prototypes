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
	
	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle Location"));
	MuzzleLocation->SetupAttachment(GunMesh);
}

bool AGunBase::CanBePickedUp(const APawn* PawnHolder) const
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

	for (int i = 0; i < HolderHistory.Num();)
	{
		const FHolderHistoryData& History = HolderHistory[i];
		float Timer = History.HeldCooldownTimer;
		Timer -= DeltaTime;
		if (Timer <= 0.0f)
		{
			HolderHistory.RemoveAt(i);
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

void AGunBase::SetHolder(APawn* HolderPawn)
{
	if (!HolderPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("GunBase: NO HOLDER GIVEN. Please pass a valid holder. If you're trying to remove the holder, use RemoveHolder"));
		return;
	}
	
	UE_LOG(LogTemp, Display, TEXT("Setting holder of gun"));
	
	this->Holder = HolderPawn;
	SetOwner(HolderPawn);
	
	//Disable collision while holding
	SetPhysicsEnabled(false);
	
	const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	if (const ACharacter* Character = Cast<ACharacter>(HolderPawn))
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
	if (!HolderPawn) return;
	
	UE_LOG(LogTemp, Display, TEXT("Removing holder from gun"));
	
	this->Holder = nullptr;
	SetOwner(nullptr);
	
	SetPhysicsEnabled(true);
	
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
		
		FHolderHistoryData History;
		History.PreviousHolder = HolderPawn;
		History.HeldCooldownTimer = 0.5f;
		
		HolderHistory.Add(History);
	}
}

