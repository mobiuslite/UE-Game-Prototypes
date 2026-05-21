#pragma once

#include "CoreMinimal.h"
#include "BoxelPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "MobiusAbilitySystem/Player/MAPlayerController.h"
#include "BoxelPlayerController.generated.h"

class UDeathBringerItemDataAsset;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class BOXEL_API ABoxelPlayerController : public AMAPlayerController
{
	GENERATED_BODY()
public:
	
	ABoxelPlayerController(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(BlueprintAssignable)
	FOnSpeakingChangedSignature OnSpeakingChanged;
	
	UFUNCTION(Reliable, Client)
	void ShowSpleefModifierName(const FString& Name);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowSpleefModifierName(const FString& Name);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void RequestItemPurchase(const UDeathBringerItemDataAsset* Data);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void LeaveLobbyArena();
	
protected:
	virtual void BeginPlay() override;
	
	virtual void BeginSpectatingState() override;
	virtual void EndSpectatingState() override;
	
	virtual ASpectatorPawn* SpawnSpectatorPawn() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TSoftObjectPtr<UInputMappingContext> InputMapping;
private:
	
	FVector UnpossessPawnLocation;
};
