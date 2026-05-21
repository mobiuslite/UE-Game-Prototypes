// 


#include "Gameplay/Weapons/GunBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
#include "MobiusAbilitySystem/Utils/MAGameplayTags.h"
#include "MobiusAbilitySystem/Utils/MAUtils.h"
#include "Utility/MobiusGameplayTags.h"
#include "Utility/MobiusUtils.h"

AGunBase::AGunBase()
{
	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle Location"));
	
	UStaticMeshComponent* Mesh = GetItemMesh();
	ensure(Mesh != nullptr);
	
	MuzzleLocation->SetupAttachment(Mesh);
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
	const bool bIsGuilty = UMAUtils::HasLooseGameplayTagEX(GetHolder(), TAG_DeathBringer_Guilty);
	const float MaxSpread = bIsGuilty ? MaxBulletSpread + GuiltyAdditiveSpreadAmount : MaxBulletSpread;
	
	BulletSpreadAmount = FMath::Clamp(BulletSpreadAmount + SpreadPerBullet, 0.0f, MaxSpread);
	
	if (ABoxelPlayerCharacter* Player = Cast<ABoxelPlayerCharacter>(GetHolder()))
	{
		Player->AddRecoil(RecoilPerBullet, MaxRecoil);
	}
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
		const UCharacterMovementComponent* MovementComp = Cast<UCharacterMovementComponent>(Holder->GetMovementComponent());
		const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Holder);
		
		if (MovementComp && ASC)
		{
			const float MaxSpread = ASC->GetNumericAttributeBase(UMACommonAttributeSet::GetMoveSpeedAttribute()) * 0.01f;
			float CurrentSpread = MaxSpread;
			
			if (!MovementComp->IsFalling())
			{
				const float Alpha = FMath::Clamp(MovementComp->Velocity.Length() * 0.01f / MaxSpread, 0.0f, 1.0f);
				CurrentSpread = UKismetMathLibrary::Ease(0.0f, MaxSpread, Alpha, EEasingFunc::EaseIn, MovementSpreadEaseExp);
			}
			
			Result += CurrentSpread * MovementSpreadMultiplier;
		}
		
		if (UMAUtils::HasLooseGameplayTagEX(Holder, TAG_DeathBringer_Guilty))
		{
			Result += GuiltyAdditiveSpreadAmount;
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
	
	bool bCheckResource = true;
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Holder))
	{
		if (ASC->HasMatchingGameplayTag(TAG_Resources_IgnoreCost))
		{
			bCheckResource = false;
		}
	}
	
	if (bCheckResource && Inventory->GetResourceCount(AmmoResourceTag) == 0) return false;
	
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
	
	if (GetItemMesh())
	{
		GetItemMesh()->SetPhysicsLinearVelocity(FVector(0.0f));
		GetItemMesh()->SetPhysicsAngularVelocityInDegrees(FVector(0.0f));
	}
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
		bool bShouldConsume = true;
		
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetHolder()))
		{
			if (ASC->HasMatchingGameplayTag(TAG_Resources_IgnoreCost))
			{
				bShouldConsume = false;
			}
		}
		
		if (bShouldConsume)
		{
			Inventory->ConsumeResource(AmmoResourceTag, 1);
		}
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
	
	bool bCheckResource = true;
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Holder))
	{
		if (ASC->HasMatchingGameplayTag(TAG_Resources_IgnoreCost))
		{
			bCheckResource = false;
		}
	}
	
	if (bCheckResource && Inventory->GetResourceCount(AmmoResourceTag) == 0) return;
	
	ReloadTimer = ReloadDuration;
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
	Super::OnEquip_Implementation(HolderController);
	
	const float ServerWorldTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	EquipReadyWorldTime = ServerWorldTime + EquipTime;
}

void AGunBase::OnUnequip_Implementation(AController* HolderController)
{
	Super::OnUnequip_Implementation(HolderController);
	
	CancelReloading();
}

float AGunBase::GetDamageAmount(const float Distance) const
{
	if (!DamageFalloff) return  BaseDamageAmount;
	
	float DamageMultiplier = DamageFalloff->GetFloatValue(Distance);
	if (UMAUtils::HasLooseGameplayTagEX(GetHolder(), TAG_DeathBringer_Guilty))
	{
		DamageMultiplier *= GuiltyDamageMultiplier;
	}
	
	return BaseDamageAmount * DamageMultiplier;
}

bool AGunBase::CanUseGun() const
{
	const float ServerWorldTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	if (EquipReadyWorldTime > ServerWorldTime) return false;
	
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

