// 2026 Mobius Lite Games (C) All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UMobiusProjMovementComponent;

UCLASS()
class BOXEL_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectile();

protected:
	
	UPROPERTY(EditDefaultsOnly)
	bool bCanDealWorldDamage = true;
	UPROPERTY(EditDefaultsOnly)
	bool bDestroyOnImpact = true;
	UPROPERTY(EditDefaultsOnly)
	bool bDestroyClientDuplicateProjectile = true;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMobiusProjMovementComponent* MovementComponent;
	
	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);
	UFUNCTION()
	bool IsClientPredictedProjectile() const;
	UFUNCTION()
	bool IsServerDuplicateProjectile() const;
	
	UFUNCTION(BlueprintNativeEvent)
	void AUTH_ProjectileStop(const FHitResult& ImpactResult);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
