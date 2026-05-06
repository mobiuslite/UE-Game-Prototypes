// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponSpawner.generated.h"

class AGunBase;

USTRUCT()
struct FWeaponSpawnData
{
	GENERATED_BODY()
	
	UPROPERTY()
	AActor* Actor;
	
	bool bIsRespawnable;
	TSubclassOf<AActor> Class;
	
	FVector SpawnLocation;
};

UCLASS()
class BOXEL_API AWeaponSpawner : public AActor
{
	GENERATED_BODY()

public:
	AWeaponSpawner();

	//Resets the weapons back to their original spawn point
	void ResetSpawner();
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AGunBase> GunClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ResourceClass;
	
	UPROPERTY(EditAnywhere)
	int NumResources;
	
	UPROPERTY(EditAnywhere)
	float SpreadAmount = 50.0f;
	
	UFUNCTION()
	void OnRoundReset();
	
private:
	
	UPROPERTY()
	TArray<FWeaponSpawnData> SpawnedActors;
};
