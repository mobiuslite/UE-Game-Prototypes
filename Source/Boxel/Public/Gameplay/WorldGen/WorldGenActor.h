#pragma once

#include "CoreMinimal.h"
#include "WorldGeneration/BoxelWorld.h"
#include "WorldGenActor.generated.h"

UCLASS(Blueprintable, BlueprintType)
class BOXEL_API AWorldGenActor : public ABoxelWorld
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWorldGenActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
};
