// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerSwitch.generated.h"

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
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int PowerSwitchID = -1;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Powered)
	bool bPowered;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnRep_Powered();
	
	UFUNCTION()
	void AUTH_OnRoundReset();
};
