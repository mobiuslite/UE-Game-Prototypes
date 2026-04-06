// 


#include "Gameplay/AbilitySystem/MuzzleCueNotify_Static.h"

#include "Kismet/GameplayStatics.h"

void UMuzzleCueNotify_Static::SpawnMuzzleTracer(const FVector& Location, const FVector& Direction, UParticleSystem* LegacyTraceVFX, const TSubclassOf<AActor> BaseActor) const
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* NewTraceActor = GetWorld()->SpawnActor<AActor>(BaseActor, Location, Direction.Rotation(), Params);
	
	UGameplayStatics::SpawnEmitterAttached(LegacyTraceVFX, NewTraceActor->GetRootComponent(), NAME_None, 
		FVector::Zero(), FRotator::ZeroRotator, FVector(1), EAttachLocation::SnapToTarget, true, EPSCPoolMethod::AutoRelease, true );
}
