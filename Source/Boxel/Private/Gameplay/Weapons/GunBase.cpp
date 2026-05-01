// 


#include "Gameplay/Weapons/GunBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Net/UnrealNetwork.h"
#include "Utility/CollisionConsts.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "Utility/MobiusUtils.h"


AGunBase::AGunBase()
{
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh"));
	SetRootComponent(GunMesh);
	
	GunMesh->SetCollisionObjectType(ECC_Gun);
	GunMesh->SetSimulatePhysics(true);
	
	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle Location"));
	MuzzleLocation->SetupAttachment(GunMesh);
}

float AGunBase::GetFireRate() const
{
	return (1.0f / RPM) * 60.0f;
}

bool AGunBase::GetGunZoomAmount(float& ZoomAmount) const
{
	if (!bHasScope) return false;
	
	ZoomAmount = ScopeZoomAmount;
	return true;
}

void AGunBase::ApplySpread()
{
	BulletSpreadAmount = FMath::Clamp(BulletSpreadAmount + SpreadPerBullet, 0.0f, MaxBulletSpread);
}

float AGunBase::GetTotalBulletSpread() const
{
	const APawn* Holder = GetHolder();
	
	bool bIsAiming = false;
	if (const ABoxelPlayerCharacter* Player = Cast<ABoxelPlayerCharacter>(Holder))
	{
		bIsAiming = Player->IsAiming();
	}
	
	float Result = BulletSpreadAmount + (bIsAiming && bHasScope ? AimingMinSpreadAmount : MinSpreadAmount);
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
	const APawn* Holder = GetHolder();
	
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
}

void AGunBase::OnRep_Holder()
{
	Super::OnRep_Holder();
	
	CancelReloading();
}

void AGunBase::OnHolderHistoryRemoved()
{
	GunMesh->ClearMoveIgnoreActors();
}

void AGunBase::OnHolderHistoryAdded()
{
	GunMesh->IgnoreActorWhenMoving(GetHolder(), true);
}

void AGunBase::BeginPlay()
{
	Super::BeginPlay();
	ResetGun();
}

void AGunBase::FinishedReloading_Implementation()
{
	UInventoryComponent* Inventory;
	if (UMobiusUtils::GetInventory(GetHolder()->GetPlayerState<APlayerState>(), Inventory))
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
	const APawn* Holder = GetHolder();
	
	if (!Holder) return;
	if (CurrentClipAmmo == AmmoPerClip) return;
	
	UInventoryComponent* Inventory;
	if (!UMobiusUtils::GetInventory(Holder->GetPlayerState(), Inventory)) return;
	
	if (Inventory->GetResourceCount(AmmoResourceTag) == 0) return;
	
	ReloadTimer = ReloadDuration;
}

void AGunBase::SetPhysicsEnabled(const bool bEnabled)
{
	Super::SetPhysicsEnabled(bEnabled);
	GunMesh->SetSimulatePhysics(bEnabled);
}

// Called every frame
void AGunBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (BulletSpreadAmount > 0.0f)
	{
		BulletSpreadAmount = FMath::Clamp(BulletSpreadAmount - (DeltaSeconds * SpreadReductionPerSecond), 0.0f, MaxBulletSpread);
	}
	
	if (ReloadTimer > 0.0f)
	{
		ReloadTimer -= DeltaSeconds;
		if (ReloadTimer <= 0.0f)
		{
			FinishedReloading();
		}
	}
}

void AGunBase::OnEquip_Implementation(AController* HolderController)
{
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* AbilityComp = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HolderController->GetPlayerState<APlayerState>()))
		{
			AbilityHandle = AbilityComp->K2_GiveAbility(GrantedAbilityClass, 0, 1);
		}
	}
	
	Super::OnEquip_Implementation(HolderController);
}

void AGunBase::OnUnequip_Implementation(AController* HolderController)
{
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* AbilityComp = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HolderController->GetPlayerState<APlayerState>()))
		{
			AbilityComp->ClearAbility(AbilityHandle);
			AbilityHandle = FGameplayAbilitySpecHandle();
		}
	}
	
	Super::OnUnequip_Implementation(HolderController);
	
	CancelReloading();
}

void AGunBase::OnAddedToInventory_Implementation(const UInventoryComponent* Inventory, AController* HolderController)
{
	Super::OnAddedToInventory_Implementation(Inventory, HolderController);
}

void AGunBase::OnRemovedFromInventory_Implementation(const UInventoryComponent* Inventory, AController* HolderController)
{
	UE_LOG(LogTemp, Display, TEXT("Removing holder from gun"));
	
	APawn* HolderPawn = HolderController->GetPawn();
	
	Super::OnRemovedFromInventory_Implementation(Inventory, HolderController);
	
	//TODO: Don't throw on killed death
	if (HolderPawn)
	{
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
	
	if (GetHolder())
	{
		if (const AController* HolderController = GetHolder()->GetController())
		{
			bResult = HolderController->IsLocalController();
		}
	}
	
	return bResult;
}

