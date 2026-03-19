#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

#include "MobiusAbilitySystemComponent.generated.h"


class UMACommonAttributeSet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class MOBIUSABILITYCOMPONENT_API UMobiusAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UMobiusAbilitySystemComponent();
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY()
	const UMACommonAttributeSet* AttributeSet;
	
	//Boiler Plate
public:
	virtual IAbilitySystemReplicationProxyInterface* GetReplicationInterface() override;
};
