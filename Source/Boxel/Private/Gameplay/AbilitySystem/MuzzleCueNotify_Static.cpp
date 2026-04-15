// 


#include "Gameplay/AbilitySystem/MuzzleCueNotify_Static.h"

#include "Kismet/GameplayStatics.h"

void UMuzzleCueNotify_Static::SpawnMuzzleTracer(const FVector& Location, const FVector& Direction, const TSubclassOf<AActor> BaseActor) const
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AActor>(BaseActor, Location, Direction.Rotation(), Params);
}
