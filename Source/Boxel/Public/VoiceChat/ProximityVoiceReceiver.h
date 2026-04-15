// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ProximityVoiceReceiver.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UProximityVoiceReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BOXEL_API IProximityVoiceReceiver
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ReceiveVoiceData(const TArray<uint8>& Data);
};
