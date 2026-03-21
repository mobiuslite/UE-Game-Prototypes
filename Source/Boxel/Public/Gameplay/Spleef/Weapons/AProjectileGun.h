// 

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Weapons/GunBase.h"
#include "AProjectileGun.generated.h"

class AProjectile;

UCLASS()
class BOXEL_API AProjectileGun : public AGunBase
{
	GENERATED_BODY()

public:
	AProjectileGun();

protected:
	
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	TSubclassOf<AProjectile> ProjectileClass;
	
	virtual void FireProjectile(const FVector& Location, const FRotator& Rotation) override;
};
