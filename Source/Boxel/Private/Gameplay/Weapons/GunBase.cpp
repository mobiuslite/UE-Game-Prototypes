// 


#include "Gameplay/Weapons/GunBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Net/UnrealNetwork.h"
#include "Utility/CollisionConsts.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/DeathBringer/InventoryComponent.h"
#include "Gameplay/Interfaces/InventoryInterface.h"
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

void AGunBase::ApplySpread()
{
	BulletSpreadAmount = FMath::Clamp(BulletSpreadAmount + SpreadPerBullet, 0.0f, MaxBulletSpread);
}

float AGunBase::GetTotalBulletSpread() const
{
	float Result = BulletSpreadAmount;
	if (Holder)
	{
		if (const UCharacterMovementComponent* MovementComp = Cast<UCharacterMovementComponent>(Holder->GetMovementComponent()))
		{
			Result += (MovementComp->IsFalling() ? MovementComp->GetMaxSpeed() : MovementComp->Velocity.Length()) * (MovementSpreadMultiplier * 0.01f);
		}
	}
	
	return Result;
}

void AGunBase::ConsumeAmmo()
{
	CurrentClipAmmo--;
	OnAmmoChangedDelegate.Broadcast(CurrentClipAmmo, AmmoPerClip);
}

bool AGunBase::StartReload_Implementation()
{
	if (ReloadTimer > 0.0f) return false;
	if (!Holder) return false;
	if (CurrentClipAmmo == AmmoPerClip) return false;
	
	UInventoryComponent* Inventory;
	if (!UMobiusUtils::GetInventory(Holder->GetPlayerState(), Inventory)) return false;
	
	if (Inventory->GetResourceCount(AmmoResourceTag) == 0) return false;
	
	ReloadTimer = ReloadDuration;
	
	if (!HasAuthority())
	{
		Server_RequestReload();
	}
	
	return true;
}

void AGunBase::ResetGun()
{
	CurrentClipAmmo = AmmoPerClip;
	ReloadTimer = 0.0f;
}

void AGunBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentClipAmmo);
	DOREPLIFETIME(ThisClass, Holder);
}

void AGunBase::BeginPlay()
{
	Super::BeginPlay();
	ResetGun();
}

void AGunBase::OnRep_Holder()
{
	SetOwner(Holder);
	
	//Disable collision while holding
	SetPhysicsEnabled(Holder == nullptr);
	
	CancelReloading();
	
	if (Holder)
	{
		const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
		if (const ACharacter* Character = Cast<ACharacter>(Holder))
		{
			AttachToComponent(Character->GetMesh(), Rules, FName("GunSocket"));
		}
		else
		{
			AttachToActor(Holder, Rules);	
		}
	}
	else
	{
		const FDetachmentTransformRules Rules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, false); 
		DetachFromActor(Rules);
		
		//Rotate gun so the side is facing the player that dropped it
		GunMesh->AddWorldRotation(FRotator(0.0f, 45.0f, 0.0f));
	}
}

void AGunBase::FinishedReloading_Implementation()
{
	UInventoryComponent* Inventory;
	if (UMobiusUtils::GetInventory(Holder->GetPlayerState<APlayerState>(), Inventory))
	{
		Inventory->ConsumeResource(AmmoResourceTag, 1);
	}
	
	CurrentClipAmmo = AmmoPerClip;
	OnAmmoChangedDelegate.Broadcast(CurrentClipAmmo, AmmoPerClip);
}

void AGunBase::CancelReloading_Implementation()
{
	ReloadTimer = 0.0f;
}

void AGunBase::Server_RequestReload_Implementation()
{
	if (!Holder) return;
	if (CurrentClipAmmo == AmmoPerClip) return;
	
	UInventoryComponent* Inventory;
	if (!UMobiusUtils::GetInventory(Holder->GetPlayerState(), Inventory)) return;
	
	if (Inventory->GetResourceCount(AmmoResourceTag) == 0) return;
	
	ReloadTimer = ReloadDuration;
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
	
	if (BulletSpreadAmount > 0.0f)
	{
		BulletSpreadAmount = FMath::Clamp(BulletSpreadAmount - (DeltaTime * SpreadReductionPerSecond), 0.0f, MaxBulletSpread);
	}
	
	if (ReloadTimer > 0.0f)
	{
		ReloadTimer -= DeltaTime;
		if (ReloadTimer <= 0.0f)
		{
			FinishedReloading();
		}
	}
}

void AGunBase::OnEquip(AController* HolderController)
{
	if (!HasAuthority()) return;
		
	if (UAbilitySystemComponent* AbilityComp = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HolderController->GetPlayerState<APlayerState>()))
	{
		AbilityComp->K2_GiveAbility(GrantedAbilityClass, 0, 1);
	}
}

void AGunBase::OnUnequip(AController* HolderController)
{
	if (!HasAuthority()) return;
	
	if (UAbilitySystemComponent* AbilityComp = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HolderController->GetPlayerState<APlayerState>()))
	{
		AbilityComp->ClearAllAbilitiesWithInputID(1);
	}
}

void AGunBase::OnAddedToInventory(const UInventoryComponent* Inventory, AController* HolderController)
{
	this->Holder = HolderController->GetPawn();
	OnRep_Holder();
}

void AGunBase::OnRemovedFromInventory(const UInventoryComponent* Inventory, AController* HolderController)
{
	UE_LOG(LogTemp, Display, TEXT("Removing holder from gun"));
	
	this->Holder = nullptr;
	OnRep_Holder();
	
	//TODO: Don't throw on killed death
	if (const APawn* HolderPawn = HolderController->GetPawn())
	{
		FHolderHistoryData History;
		History.PreviousHolder = HolderPawn;
		History.HeldCooldownTimer = 0.5f;
		
		HolderHistory.Add(History);
		
		GunMesh->AddImpulse(HolderPawn->GetBaseAimRotation().Vector() * DropImpulseStrength, NAME_None, true);
	}
}

bool AGunBase::CanUseGun() const
{
	return ReloadTimer <= 0.0f && CurrentClipAmmo > 0;
}

bool AGunBase::IsLocallyHeldGun() const
{
	bool bResult = false;
	
	if (Holder)
	{
		if (const AController* HolderController = Holder->GetController())
		{
			bResult = HolderController->IsLocalController();
		}
	}
	
	return bResult;
}

