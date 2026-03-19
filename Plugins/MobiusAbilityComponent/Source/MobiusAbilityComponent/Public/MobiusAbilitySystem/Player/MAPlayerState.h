#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "MAPlayerState.generated.h"

class UMobiusAbilitySystemComponent;

UCLASS()
class MOBIUSABILITYCOMPONENT_API AMAPlayerState : public APlayerState,
	public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	
	AMAPlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UMobiusAbilitySystemComponent* AbilityComponent;
	
};
