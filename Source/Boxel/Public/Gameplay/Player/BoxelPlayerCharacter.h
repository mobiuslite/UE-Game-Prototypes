#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GenericTeamAgentInterface.h"
#include "Gameplay/Interfaces/Interactable.h"
#include "MobiusAbilitySystem/Player/MACharacter.h"
#include "BoxelPlayerCharacter.generated.h"

class ABoxelPlayerState;
class UCameraComponent;
class AInventoryItem;
class UGameplayAbility;
class UInventoryComponent;
class UToastWidget;
class USphereComponent;
class AGunBase;
class AProjectile;
struct FInputActionValue;
class UInputAction;

USTRUCT(BlueprintType)
struct FDeadPlayerInfo
{
	GENERATED_BODY()
	
	FDeadPlayerInfo()
	{
		TeamId = FGenericTeamId::NoTeam;
	}
	
	UPROPERTY(BlueprintReadOnly)
	int TeamId;
	
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;
	
	UPROPERTY(BlueprintReadOnly)
	FHitResult KillshotInfo;
	UPROPERTY(BlueprintReadOnly)
	FString KilledByWeaponName;
};


UENUM(BlueprintType)
namespace EGenericPlayerMessage
{
	enum Type : uint8
	{
		None,
	
		Guilty,
	
		COUNT
	};
}

USTRUCT()
struct FCameraFOVEffect
{
	GENERATED_BODY()
	float Timer;
	float Strength;
};

UCLASS()
class BOXEL_API ABoxelPlayerCharacter : public AMACharacter, public IInteractable
{
	GENERATED_BODY()

public:
	ABoxelPlayerCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void Landed(const FHitResult& Hit) override;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetFireInputId() const { return 1; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetCrouchInputId() const { return 2; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AInventoryItem* GetHeldItem() const { return HeldItem; }
	void SetPlayerUnarmed();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsAiming() const { return AimToastId != INDEX_NONE; }
	
	UFUNCTION()
	bool PickUpItem(AInventoryItem* Item, const bool bConceal = false);
	UFUNCTION(Server, Reliable)
	void DropHeldGun(const bool bThrow = true);
	UFUNCTION(BlueprintCallable)
	void DropAllItems();
	
	UPROPERTY(BlueprintReadWrite)
	FRotator ExtraViewRotation;
	
	UFUNCTION()
	void AddRecoil(const float Amount, const float MaxRecoilAmount);
	
	virtual FRotator GetViewRotation() const override;

	//Visuals only
	virtual void Client_OnDamageTaken_Implementation(const AController* DamageInstigator, const AActor* DamageCauser, const bool bIsDead) override;
	UFUNCTION(BlueprintImplementableEvent)
	void BP_LocalOnTakeDamage(const AController* DamageInstigator, const AActor* DamageCauser, const bool bIsDead);
	
	//Player death logic
	virtual void Server_OnPlayerDead(const FHitResult& Hit, const AController* DamageInstigator, const AActor* DamageCauser) override;
	virtual void Client_KilledPlayer_Implementation(const APlayerState* KilledPlayerState) override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void BP_KilledPlayer(const APlayerState* KilledPlayerState);
	
	UFUNCTION(NetMulticast, Reliable)
	void EnableRagdoll(const FHitResult& Hit);
	
	//Interactable
	virtual bool CanInteract_Implementation() const override;
	virtual void Interact_Implementation(APawn* Caller) override;
	virtual FText GetInteractPreviewString_Implementation() const override;
	
	UFUNCTION(Server, Reliable)
	void Server_PlayerInteracted(UObject* Interactable);
	
	UFUNCTION(BlueprintCallable)
	void AddFOVEffect(const float Strength, const float Duration, const bool bInstantSet = false);
	
	//Because multiple things can make the player walk (aiming, crouching)
	//We need to have a tracker of how many things "RequestWalk" has been called
	//This is so when the player crouches and aims, letting go of aim doesnt cause the player to 
	//stop walking
	UFUNCTION(BlueprintCallable)
	void RequestWalk();
	UFUNCTION(BlueprintCallable)
	void RemoveWalkRequest();
	
	UFUNCTION(Client, Reliable)
	void Client_SendGenericMessage(const EGenericPlayerMessage::Type& Message, const TArray<uint8>& Payload);
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnRoundReset();
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnLocalPlayerStateReady(ABoxelPlayerState* LocalPlayerState);
	UFUNCTION(BlueprintNativeEvent)
	void OnProxyPlayerStateReady(ABoxelPlayerState* ProxyPlayerState);
	
	UFUNCTION(BlueprintNativeEvent)
	void OnTriggerPressed();
	UFUNCTION(BlueprintNativeEvent)
	void OnTriggerReleased();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnAimPressed();
	UFUNCTION(BlueprintNativeEvent)
	void OnAimReleased();

	UFUNCTION()
	void OnItemOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	//If the pawn wasn't relevant when killed, we still want the player to ragdoll
	UPROPERTY(ReplicatedUsing=OnRep_IsRagdoll);
	bool bIsRagdoll;
	UFUNCTION()
	void OnRep_IsRagdoll();
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_HeldItem)
	AInventoryItem* HeldItem;
	UFUNCTION(BlueprintNativeEvent)
	void OnRep_HeldItem(const AInventoryItem* LastGun);
	UFUNCTION(Server, Reliable)
	void Server_SetHeldItemIndex(const int Index);
	void SetHeldItemIndex(int Index);
	
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Weapons")
	TSubclassOf<AGunBase> StartingGunClass;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Weapons")
	float RecoilRecoverDuration = 0.67f;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Weapons")
	float RecoilLerpSpeed = 7.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|UI")
	float PreviewPlayerInfoDistance = 300.0f;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|UI")
	TSubclassOf<UToastWidget> PreviewPlayerToastClass;
	
	//Used for displaying player name and damage while they're still alive
	void DoPlayerPreviewTrace();
	int PreviewToastId = -1;
	
	UFUNCTION(Client, Reliable)
	void Client_ShowDeadPlayerInfo(const FDeadPlayerInfo& Info);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowDeadPlayerInfo(const FDeadPlayerInfo& Info);
	UPROPERTY(BlueprintReadOnly)
	FDeadPlayerInfo DeadPlayerInfo;
	
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Interact")
	float PlayerInteractDistance = 300.0f;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Interact")
	TSubclassOf<UToastWidget> InteractToastClass;
	
	UPROPERTY(BlueprintReadOnly)
	TScriptInterface<IInteractable> CurrentInteractable;
	
	void DoInteractTrace();
	int InteractToastId = -1;
	
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Ragdoll")
	float RagdollPushbackStrength = 350.0f;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Ragdoll")
	float RagdollRotationStrength = 60.0f;
	UPROPERTY(EditAnywhere, Category="BoxelPlayerCharacter|Ragdoll")
	float HeadshotHeight = 140.0f;
	UPROPERTY(EditAnywhere, Category="BoxelPlayerCharacter|Ragdoll")
	float LegShotHeight = 70.0f;
	
	//Input
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* MoveAction;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* LookAction;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* JumpAction;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* CrouchAction;
	
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* FireAction;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* AimAction;
	
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* DropAction;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* InteractAction;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* ReloadAction;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* ScrollInventoryAction;
	
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* PushToTalkAction;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* TeamTalkAction;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* PauseAction;
	//Input
	
	//A new on landed method with extra info
	UFUNCTION(BlueprintNativeEvent)
	void OnLandedEX(const float ZVelocity, const FHitResult& Hit);
	
	UPROPERTY(BlueprintReadOnly)
	bool bRadioTalking;
	UPROPERTY(BlueprintReadOnly)
	bool bTeamTalking;
	
	int AimToastId = -1;
	
	UPROPERTY()
	TSubclassOf<UAnimInstance> OriginalAnimInstanceClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> UnarmedAbilityClass;
	UPROPERTY()
	FGameplayAbilitySpecHandle UnarmedAbilityHandle;
	
	UPROPERTY()
	UCameraComponent* PlayerCamera;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Camera")
	float FOVChangeSpeed = 5.0f;
	
	float RecoilTarget;
	float CurrentRecoilAmount;
	float RecoilRecoverTimer;
	
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnGenericMessageReceived(const TEnumAsByte<EGenericPlayerMessage::Type>& Message, const TArray<uint8>& Payload);
	
private:
	
	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);
	
	void JumpInput(const FInputActionValue& Value);
	void JumpInput_Released(const FInputActionValue& Value);
	
	void CrouchInput(const FInputActionValue& Value);
	void CrouchInput_Released(const FInputActionValue& Value);
	
	void FireInput(const FInputActionValue& Value);
	void FireInput_Released(const FInputActionValue& Value);
	
	void AimInput(const FInputActionValue& Value);
	void AimInput_Released(const FInputActionValue& Value);
	
	void DropInput(const FInputActionValue& Value);
	void InteractInput(const FInputActionValue& Value);
	void ReloadInput(const FInputActionValue& Value);
	
	void ScrollInventoryInput(const FInputActionValue& Value);
	
	void TalkInput(const FInputActionValue& Value);
	void TalkInput_Released(const FInputActionValue& Value);

	void TeamTalkInput(const FInputActionValue& Value);
	void TeamTalkInput_Released(const FInputActionValue& Value);
	
	void PauseInput(const FInputActionValue& Value);
	
	float CurrentZoomAmount = 1.0f;
	void CleanupToasts();
	
	void SetSpeaking(const bool bSpeaking, const bool bTeamSpeaking);
	
	UPROPERTY()
	TArray<FCameraFOVEffect> FOVEffects;
	
	float CurrentCameraFOV;
	int WalkRequests;
	
public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
