#pragma once

#include "CoreMinimal.h"
#include "MobiusAbilitySystem/Player/MAPlayerState.h"
#include "BoxelPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeakingChangedSignature, const bool, bSpeaking);

UCLASS()
class BOXEL_API ABoxelPlayerState : public AMAPlayerState
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable)
	void SetSpeaking(const bool bSpeaking);
	
protected:
	
	UPROPERTY(BlueprintAssignable)
	FOnSpeakingChangedSignature OnSpeakingChanged;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnSetSpeaking(const bool bSpeaking);
};
