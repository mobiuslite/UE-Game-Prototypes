// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeamTester.generated.h"

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
	void AddPawnToEvaluator(const APawn* Pawn);
	
	//returns true if successfully started, not related to test result
	UFUNCTION(BlueprintCallable)
	bool StartEvaluation();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPowered() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetNumEvaluatorsAdded() const { return PlayersToEvaluate.Num(); }
	
	UPROPERTY(BlueprintAssignable)
	FOnEvaluatedSignature OnEvaluated;
	UPROPERTY(BlueprintAssignable)
	FOnEvaluatorAddedSignature OnEvaluatorAdded;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void DisplayResults(const bool bEvaluatorsSafe);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_DisplayResults(const bool bEvaluatorsSafe);
	
	//Returns true if evaluators are all team 1, returns false if potential team 2
	UFUNCTION(BlueprintNativeEvent)
	bool Evaluate();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bRequiresPower;
	UPROPERTY(ReplicatedUsing=OnRep_Powered)
	bool bPowered;
	UFUNCTION(BlueprintNativeEvent)
	void OnRep_Powered();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int NumEvaluators = 1;
	UPROPERTY(BlueprintReadOnly)
	TArray<ABoxelPlayerState*> PlayersToEvaluate;
};
