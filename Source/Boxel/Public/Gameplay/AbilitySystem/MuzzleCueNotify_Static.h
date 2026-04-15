// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "MuzzleCueNotify_Static.generated.h"

/**
 * 
 */
UCLASS()
class BOXEL_API UMuzzleCueNotify_Static : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable)
	void SpawnMuzzleTracer(const FVector& Location, const FVector& Direction, const TSubclassOf<AActor> BaseActor) const;
};
