// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeamTester.generated.h"

class APowerSwitch;
class ABoxelPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEvaluatedSignature, const bool, bEvaluatorsSafe);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEvaluatorAddedSignature, const APawn*, Pawn);

UCLASS()
class BOXEL_API ATeamTester : public AActor
{
	GENERATED_BODY()

public:
	ATeamTester();

	UFUNCTION(BlueprintCallable)
	bool AddPawnToEvaluator(const APawn* Pawn);
	
	//returns true if successfully started, not related to test result
	UFUNCTION(BlueprintCallable)
	bool StartEvaluation();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPowered() const;
	void SetPowerSwitch(const TObjectPtr<APowerSwitch> Switch);
	int GetPowerSwitchID() const { return PowerSwitchID; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetNumEvaluatorsAdded() const { return PlayersToEvaluate.Num(); }
	
	UPROPERTY(BlueprintAssignable)
	FOnEvaluatedSignature OnEvaluated;
	UPROPERTY(BlueprintAssignable)
	FOnEvaluatorAddedSignature OnEvaluatorAdded;
protected:
	
	virtual void BeginPlay() override;
	UFUNCTION()
	void AUTH_OnRoundReset();
	
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void DisplayResults(const bool bEvaluatorsSafe);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_DisplayResults(const bool bEvaluatorsSafe);
	
	//Returns true if evaluators are all team 1, returns false if potential team 2
	UFUNCTION(BlueprintNativeEvent)
	bool Evaluate();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bRequiresPower;
	
	//Connects to power switches with this ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int PowerSwitchID = -1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int NumEvaluators = 1;
	UPROPERTY(BlueprintReadOnly)
	TArray<ABoxelPlayerState*> PlayersToEvaluate;
	
private:
	UPROPERTY()
	TObjectPtr<APowerSwitch> ConnectedSwitch;
};
