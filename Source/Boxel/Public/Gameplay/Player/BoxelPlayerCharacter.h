#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MobiusAbilitySystem/Player/MACharacter.h"
#include "BoxelPlayerCharacter.generated.h"

class AProjectile;
struct FInputActionValue;
class UInputAction;

UCLASS()
class BOXEL_API ABoxelPlayerCharacter : public AMACharacter
{
	GENERATED_BODY()

public:
	ABoxelPlayerCharacter(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(BlueprintReadWrite)
	FRotator ExtraViewRotation;
	virtual FRotator GetViewRotation() const override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnTriggerPressed();
	UFUNCTION(BlueprintNativeEvent)
	void OnTriggerReleased();
	UPROPERTY(BlueprintReadOnly)
	bool bTriggerDown;
	
	float FireTimer;
	
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
	UInputAction* PushToTalkAction;
	UPROPERTY(EditDefaultsOnly, Category="BoxelPlayerCharacter|Input|Actions")
	UInputAction* PauseAction;
	//Input
	
	float TimeSpentInAir;
	virtual void Landed(const FHitResult& Hit) override;
	
	//A new on landed method with extra info, like time spend in air
	UFUNCTION(BlueprintNativeEvent)
	void OnLandedEX(const float TimeInAir, const FHitResult& Hit);
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ProjectileClass;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnRep_Talking();
	UPROPERTY(ReplicatedUsing=OnRep_Talking, BlueprintReadOnly)
	bool bTalking;
private:
	
	void FireProjectile(const FVector& Location, const FRotator& Rotation);
	
	UFUNCTION(Server, Reliable)
	void Server_FireProjectile(const FVector& Location, const FRotator& Rotation);
	
	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);
	
	void JumpInput(const FInputActionValue& Value);
	void JumpInput_Released(const FInputActionValue& Value);
	
	void FireInput(const FInputActionValue& Value);
	void FireInput_Released(const FInputActionValue& Value);
	
	void TalkInput(const FInputActionValue& Value);
	void TalkInput_Released(const FInputActionValue& Value);

	void PauseInput(const FInputActionValue& Value);
	
public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
