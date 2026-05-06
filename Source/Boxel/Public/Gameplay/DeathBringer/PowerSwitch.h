// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerSwitch.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPowerStateChangedSignature, const bool, bPowered);

UCLASS()
class BOXEL_API APowerSwitch : public AActor
{
	GENERATED_BODY()

public:
	APowerSwitch();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPowered() const { return bPowered; }
	UFUNCTION(BlueprintCallable)
	void SetPowered(const bool bPower);
	
	UFUNCTION(BlueprintCallable)
	void SetBroken(const bool bBroke);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsBroken() const { return bBroken; }
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(BlueprintAssignable)
	FOnPowerStateChangedSignature OnPoweredStateChanged;
	
protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Powered)
	bool bPowered;
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bBroken;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnRep_Powered();
	
	UFUNCTION()
	void OnRoundReset();
};
