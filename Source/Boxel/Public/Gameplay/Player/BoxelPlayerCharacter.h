#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Interfaces/Interactable.h"
#include "MobiusAbilitySystem/Player/MACharacter.h"
#include "BoxelPlayerCharacter.generated.h"

class IInventoryItem;
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
	
	UPROPERTY(BlueprintReadOnly)
	int TeamId;
	
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;
	
	UPROPERTY(BlueprintReadOnly)
	FHitResult KillshotInfo;
	UPROPERTY(BlueprintReadOnly)
	FString KilledByWeaponName;
};

UCLASS()
class BOXEL_API ABoxelPlayerCharacter : public AMACharacter, public IInteractable
{
	GENERATED_BODY()

public:
	ABoxelPlayerCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void Landed(const FHitResult& Hit) override;
	
	//TODO: Add a bit more of a generic inventory system. Allow 1 side arm, and 1 main firearm
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TScriptInterface<IInventoryItem> GetHeldItem() const { return HeldItem; }
	
	UFUNCTION()
	bool PickUpItem(const TScriptInterface<IInventoryItem>& Item, const bool bConceal = false);
	UFUNCTION(Server, Reliable)
	void DropHeldGun(const bool bThrow = true);
	UFUNCTION()
	void DropAllItems();
	
	UPROPERTY(BlueprintReadWrite)
	FRotator ExtraViewRotation;
	virtual FRotator GetViewRotation() const override;

	//Visuals only
	virtual void Client_OnDamageTaken_Implementation(const AController* DamageInstigator, const AActor* DamageCauser, const bool bIsDead) override;
	UFUNCTION(BlueprintImplementableEvent)
	void BP_LocalOnTakeDamage(const AController* DamageInstigator, const AActor* DamageCauser);
	
	//Player death logic
	virtual void Server_OnPlayerDead(const FHitResult& Hit, const AActor* Causer) override;
	UFUNCTION(NetMulticast, Reliable)
	void EnableRagdoll(const FHitResult& Hit);
	
	//Interactable
	virtual bool CanInteract_Implementation() const override;
	virtual void Interact_Implementation(APawn* Caller) override;
	virtual FText GetInteractPreviewString_Implementation() const override;
	
	UFUNCTION(Server, Reliable)
	void Server_PlayerInteracted(UObject* Interactable);
	
protected:
	virtual void BeginPlay() override;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnTriggerPressed();
	UFUNCTION(BlueprintNativeEvent)
	void OnTriggerReleased();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnAimPressed();
	UFUNCTION(BlueprintNativeEvent)
	void OnAimReleased();

	UFUNCTION()
	void OnGunOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	//If the pawn wasn't relevant when killed, we still want the player to ragdoll
	UPROPERTY(ReplicatedUsing=OnRep_IsRagdoll);
	bool bIsRagdoll;
	UFUNCTION()
	void OnRep_IsRagdoll();
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_HeldItem)
	TScriptInterface<IInventoryItem> HeldItem;
	UFUNCTION()
	void OnRep_HeldItem(const TScriptInterface<IInventoryItem>& LastGun);
	UFUNCTION(Server, Reliable)
	void Server_SetHeldItemIndex(const int Index);
	void SetHeldItemIndex(int Index);
	
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Weapons")
	TSubclassOf<AGunBase> StartingGunClass;
	
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
	UInputAction* PauseAction;
	//Input
	
	float TimeSpentInAir;
	//A new on landed method with extra info, like time spend in air
	UFUNCTION(BlueprintNativeEvent)
	void OnLandedEX(const float TimeInAir, const FHitResult& Hit);
	
	UFUNCTION(BlueprintNativeEvent)
	void OnRep_Talking();
	UPROPERTY(ReplicatedUsing=OnRep_Talking, BlueprintReadOnly)
	bool bTalking;
	
	UPROPERTY()
	TSubclassOf<UAnimInstance> OriginalAnimInstanceClass;
private:
	
	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);
	
	void JumpInput(const FInputActionValue& Value);
	void JumpInput_Released(const FInputActionValue& Value);
	
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

	void PauseInput(const FInputActionValue& Value);
	
	float CurrentZoomAmount;
	
public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
