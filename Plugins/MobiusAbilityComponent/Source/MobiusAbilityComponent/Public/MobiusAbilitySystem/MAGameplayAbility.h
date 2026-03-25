// 

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MAGameplayAbility.generated.h"

UENUM(BlueprintType)
enum class EMobiusAbilityActivationPolicy : uint8
{
	Default,
	
	// Continually try to activate the ability while the input is active.
	WhileInputActive,
};

UCLASS()
class MOBIUSABILITYCOMPONENT_API UMAGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	EMobiusAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EMobiusAbilityActivationPolicy ActivationPolicy;
};
