#include "Gameplay/Weapons/Projectiles/MobiusProjMovementComponent.h"

UMobiusProjMovementComponent::UMobiusProjMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UMobiusProjMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMobiusProjMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

