// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeathRunSpawnPoint.generated.h"

UCLASS()
class BOXEL_API ADeathRunSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ADeathRunSpawnPoint();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsDeathBringerSpawnPoint() const { return bIsDeathBringerSpawn; } 
	
protected:
	
	UPROPERTY(EditAnywhere)
	bool bIsDeathBringerSpawn;
};
