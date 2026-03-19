#pragma once

#include "CoreMinimal.h"
#include "BoxelPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "MobiusAbilitySystem/Player/MAPlayerController.h"
#include "BoxelPlayerController.generated.h"

class UInputMappingContext;
/**
 * 
 */
UCLASS()
class BOXEL_API ABoxelPlayerController : public AMAPlayerController
{
	GENERATED_BODY()
public:
	
	UPROPERTY(BlueprintAssignable)
	FOnSpeakingChangedSignature OnSpeakingChanged;
	
	UFUNCTION(Reliable, Client)
	void ShowSpleefModifierName(const FString& Name);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowSpleefModifierName(const FString& Name);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TSoftObjectPtr<UInputMappingContext> InputMapping;
};
