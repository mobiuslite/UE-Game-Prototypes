
#include "Gameplay/Weapons/Projectiles/Projectile.h"

#include "Gameplay/Weapons/Projectiles/MobiusProjMovementComponent.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	MovementComponent = CreateDefaultSubobject<UMobiusProjMovementComponent>(TEXT("Projectile Movement Component"));
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	//We spawn projectiles locally for clients te reduce perceived lag. Destroy the duplicate projectile the server spawns
	if (bDestroyClientDuplicateProjectile)
	{
		if (IsServerDuplicateProjectile())
		{
			Destroy(true);
		}
	}
	
	if (!IsPendingKillPending())
	{
		MovementComponent->OnProjectileStop.AddDynamic(this, &ThisClass::OnProjectileStop);
	}
}

void AProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	if (!IsClientPredictedProjectile())
	{
		AUTH_ProjectileStop(ImpactResult);
	}
	
	if (HasAuthority() && bDestroyOnImpact)
	{
		Destroy();
	}
}

void AProjectile::AUTH_ProjectileStop_Implementation(const FHitResult& ImpactResult)
{
}

bool AProjectile::IsClientPredictedProjectile() const
{
	const bool bHasAuthority = HasAuthority();
	const ENetRole ThisRole = GetInstigator()->GetLocalRole();
	
	return bHasAuthority && ThisRole == ROLE_AutonomousProxy;
}

bool AProjectile::IsServerDuplicateProjectile() const
{
	const bool bHasAuthority = HasAuthority();
	const bool bIsLocallyControlled = Cast<APawn>(GetInstigator())->IsLocallyControlled();
	
	return !bHasAuthority && bIsLocallyControlled;
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

