#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"

#include "MobiusAbilitySystemComponent.generated.h"


class UMACommonAttributeSet;

MOBIUSABILITYCOMPONENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class MOBIUSABILITYCOMPONENT_API UMobiusAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UMobiusAbilitySystemComponent();
	
	virtual void AbilityLocalInputPressed(int32 InputID) override;
	virtual void AbilityLocalInputReleased(int32 InputID) override;
	virtual void ProcessAbilityInput(const float DeltaTime, const bool bGamePaused);
	virtual void ClearAbilityInput();
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	TArray<int32> InputsHeld;
	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY()
	const UMACommonAttributeSet* AttributeSet;
	
	//Boiler Plate
public:
	virtual IAbilitySystemReplicationProxyInterface* GetReplicationInterface() override;
};
