#pragma once

#include "CoreMinimal.h"
#include "MobiusAbilitySystem/Player/MAPlayerState.h"
#include "Team/MLTeamInterface.h"
#include "BoxelPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeakingChangedSignature, const bool, bSpeaking);

UCLASS()
class BOXEL_API ABoxelPlayerState : public AMAPlayerState, public IMLTeamAgentInterface
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable)
	void SetSpeaking(const bool bSpeaking);
	
	UFUNCTION(BlueprintCallable)
	void AUTH_SetTeamId(const FGenericTeamId& NewTeamID);

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	virtual FOnTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override { return &OnTeamChanged; }
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	
	UPROPERTY(BlueprintAssignable)
	FOnSpeakingChangedSignature OnSpeakingChanged;
	UPROPERTY(BlueprintAssignable)
	FOnTeamIndexChangedDelegate OnTeamChanged;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnSetSpeaking(const bool bSpeaking);
	
private:
	
	UFUNCTION()
	void OnRep_TeamID(const FGenericTeamId OldTeamID);
	
	UPROPERTY(ReplicatedUsing=OnRep_TeamID)
	FGenericTeamId TeamID;
};
