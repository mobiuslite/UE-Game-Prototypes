// 

#pragma once

#include "CoreMinimal.h"
#include "MobiusGameMode.h"
#include "DeathRunGameMode.generated.h"

class ADeathRunSpawnPoint;
/**
 * 
 */
UCLASS()
class BOXEL_API ADeathRunGameMode : public AMobiusGameMode
{
	GENERATED_BODY()
	
public:
	
	virtual void BeginPlay() override;
	
	virtual void StartGame() override;
	
protected:
	
	UPROPERTY(BlueprintReadOnly)
	APlayerController* CurrentDeathBringer;
	
	UPROPERTY()
	TArray<APlayerController*> PreviousDeathBringers; 
	
private:
	UPROPERTY()
	ADeathRunSpawnPoint* DeathBringerSpawnPoint;
	UPROPERTY()
	ADeathRunSpawnPoint* PlayerSpawnPoint;
};
