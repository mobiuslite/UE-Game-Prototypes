#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Interfaces/InventoryInterface.h"
#include "MobiusAbilitySystem/Player/MAPlayerState.h"
#include "Team/MLTeamInterface.h"
#include "BoxelPlayerState.generated.h"

class AGunBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeakingChangedSignature, const bool, bSpeaking);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunEquippedSignature, const AGunBase*, Gun);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunUnequippedSignature, const AGunBase*, Gun);

UCLASS()
class BOXEL_API ABoxelPlayerState : public AMAPlayerState, public IMLTeamAgentInterface, public IInventoryInterface
{
	GENERATED_BODY()

public:
	
	ABoxelPlayerState();
	
	UFUNCTION(BlueprintCallable)
	void SetSpeaking(const bool bSpeaking);
	
	UFUNCTION(BlueprintCallable)
	void AUTH_SetTeamId(const FGenericTeamId& NewTeamID);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FGenericTeamId GetTeamId() const { return GetGenericTeamId(); }

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	virtual FOnTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override { return &OnTeamChanged; }
	
	void BroadcastGunEquipped(const AGunBase* GunBase) const;
	void BroadcastGunUnequipped(const AGunBase* GunBase) const;
	
	UPROPERTY()
	UInventoryComponent* InventoryComponent;
	virtual UInventoryComponent* GetInventory() const override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	
	UPROPERTY(BlueprintAssignable)
	FOnSpeakingChangedSignature OnSpeakingChanged;
	UPROPERTY(BlueprintAssignable)
	FOnTeamIndexChangedDelegate OnTeamChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnGunEquippedSignature OnGunEquippedDelegate;
	UPROPERTY(BlueprintAssignable);
	FOnGunEquippedSignature OnGunUnequippedDelegate;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnSetSpeaking(const bool bSpeaking);
	
private:
	
	UFUNCTION()
	void OnRep_TeamID(const FGenericTeamId OldTeamID);
	
	UPROPERTY(ReplicatedUsing=OnRep_TeamID)
	FGenericTeamId TeamID;
};
