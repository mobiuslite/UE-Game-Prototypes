// 


#include "Gameplay/Spleef/Weapons/AProjectileGun.h"

#include "Gameplay/Weapons/Projectiles/Projectile.h"

AProjectileGun::AProjectileGun()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AProjectileGun::FireProjectile(const FVector& Location, const FRotator& Rotation)
{
	if (!ProjectileClass) return;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetHolder();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	GetWorld()->SpawnActor<AProjectile>(ProjectileClass, Location, Rotation, SpawnParams);
}
