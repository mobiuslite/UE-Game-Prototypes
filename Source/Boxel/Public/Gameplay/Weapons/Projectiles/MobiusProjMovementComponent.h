// 2026 Mobius Lite Games (C) All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MobiusProjMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BOXEL_API UMobiusProjMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMobiusProjMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
