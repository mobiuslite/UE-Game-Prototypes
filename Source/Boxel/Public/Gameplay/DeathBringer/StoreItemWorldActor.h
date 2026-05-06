// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoreItemWorldActor.generated.h"

class ABoxelPlayerCharacter;
//An item that can be purchased from the death bringer store that spawns an actor in the world
UCLASS()
class BOXEL_API AStoreItemWorldActor : public AActor
{
	GENERATED_BODY()

public:
	AStoreItemWorldActor();

	UFUNCTION(BlueprintNativeEvent)
	void Initialize(ABoxelPlayerCharacter* Purchaser);
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
