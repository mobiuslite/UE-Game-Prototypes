// 

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerController.h"
#include "MAPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MOBIUSABILITYCOMPONENT_API AMAPlayerController : public APlayerController,
	public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};
