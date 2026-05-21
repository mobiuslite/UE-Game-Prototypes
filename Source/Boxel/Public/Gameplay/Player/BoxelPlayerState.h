#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Interfaces/InventoryInterface.h"
#include "MobiusAbilitySystem/Player/MAPlayerState.h"
#include "Team/MLTeamInterface.h"
#include "BoxelPlayerState.generated.h"

class AGunBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeakingChangedSignature, const bool, bSpeaking, const bool, bUseTeamChannel);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunEquippedSignature, const AGunBase*, Gun);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunUnequippedSignature, const AGunBase*, Gun);

UCLASS()
class BOXEL_API ABoxelPlayerState : public AMAPlayerState, public IMLTeamAgentInterface, public IInventoryInterface
{
	GENERATED_BODY()

public:
	
	ABoxelPlayerState();
	
	static constexpr uint8 DEAD_CHANNELID = 255;
	
	UFUNCTION(BlueprintCallable)
	void SetSpeaking(const bool bSpeaking, const bool bUseTeamChannel);
	UFUNCTION(BlueprintCallable)
	void RegisterVoiceChannel(const uint8 ChannelID);
	UFUNCTION(BlueprintCallable)
	void UnregisterVoiceChannel(const uint8 ChannelID);
	UFUNCTION(BlueprintCallable)
	//Unregisters from all voice channels except 0 & 1 (Proximity and global radio)
	void UnregisterUncommonVoiceChannels();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsRegisteredToVoiceChannel(const uint8 ChannelID) const;
	TArray<uint8> GetRegisteredVoiceChannels() const { return RegisteredVoiceChannels; }
	
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
	
	//We do this instead of normal replication so that people who aren't on the same team don't receive the message for it.
	UFUNCTION(Client, Reliable)
	void SetTeammates(const FGenericTeamId& Team, const TArray<ABoxelPlayerState*>& TeammatesPlayerStates);
	
	UFUNCTION(BlueprintNativeEvent)
	void OnLocalPlayerStateReady();
	UFUNCTION(BlueprintNativeEvent)
	void OnProxyPlayerStateReady();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintAssignable)
	FOnSpeakingChangedSignature OnSpeakingChanged;
	UPROPERTY(BlueprintAssignable)
	FOnTeamIndexChangedDelegate OnTeamChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnGunEquippedSignature OnGunEquippedDelegate;
	UPROPERTY(BlueprintAssignable);
	FOnGunEquippedSignature OnGunUnequippedDelegate;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnSetSpeaking(const bool bSpeaking, const bool bUseTeamChannel);
	
	UFUNCTION(Server, Reliable)
	void Server_SetIsTeamSpeaking(const bool bTeamSpeaking);
	UFUNCTION(Client, Reliable)
	void Client_SetTeammateIsSpeaking(const bool bTeamSpeaking, const APlayerState* PlayerState);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetTeammateSpeaking(const bool bSpeaking, const int64 OtherUserId, const APlayerState* PlayerState);
	
	UFUNCTION()
	void OnRoundReset();
	
private:
	
	UPROPERTY()
	TArray<uint8> RegisteredVoiceChannels; 
	
	UFUNCTION()
	void OnRep_TeamID(const FGenericTeamId OldTeamID);
	
	UPROPERTY(ReplicatedUsing=OnRep_TeamID)
	FGenericTeamId TeamID;
};
