#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "Attributes/MACommonAttributeSet.h"

#include "MobiusAbilitySystemComponent.generated.h"


MOBIUSABILITYCOMPONENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);
MOBIUSABILITYCOMPONENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Invincible);

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
	
	UFUNCTION(BlueprintCallable)
	void ResetAttributes();
	
	UPROPERTY(BlueprintAssignable)
	mutable FMAAttributeEvent OnHealthChanged;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	TArray<int32> InputsHeld;
	
	UFUNCTION()
	void OnAttributeSetHealthChanged(float EffectMagnitude, float OldValue, float NewValue);
	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY()
	const UMACommonAttributeSet* AttributeSet;
	
	//Boiler Plate
public:
	virtual IAbilitySystemReplicationProxyInterface* GetReplicationInterface() override;
};
