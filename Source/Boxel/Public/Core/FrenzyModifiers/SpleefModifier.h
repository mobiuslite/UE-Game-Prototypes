// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SpleefModifier.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class BOXEL_API USpleefModifier : public UObject
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintNativeEvent)
	void Tick(const float DeltaTime);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString ModifierName;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ApplyModifier(const TArray<APawn*>& Players);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void DestroyModifier(const TArray<APawn*>& Players);
};
