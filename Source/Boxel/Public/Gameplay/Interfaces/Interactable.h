// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class BOXEL_API IInteractable
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanInteract() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(APawn* Caller);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FText GetInteractPreviewString() const;
};
