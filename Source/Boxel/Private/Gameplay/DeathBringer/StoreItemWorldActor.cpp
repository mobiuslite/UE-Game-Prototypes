// 


#include "Gameplay/DeathBringer/StoreItemWorldActor.h"

AStoreItemWorldActor::AStoreItemWorldActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStoreItemWorldActor::Initialize_Implementation(ABoxelPlayerCharacter* Purchaser)
{
}

// Called when the game starts or when spawned
void AStoreItemWorldActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AStoreItemWorldActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}