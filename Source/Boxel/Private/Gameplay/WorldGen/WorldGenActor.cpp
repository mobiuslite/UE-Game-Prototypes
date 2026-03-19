
#include "Gameplay/WorldGen/WorldGenActor.h"

AWorldGenActor::AWorldGenActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AWorldGenActor::BeginPlay()
{
	Super::BeginPlay();
	
	GenerateWorld(0);
}

void AWorldGenActor::Destroyed()
{
	Super::Destroyed();
	ClearChunks();
}

void AWorldGenActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}
