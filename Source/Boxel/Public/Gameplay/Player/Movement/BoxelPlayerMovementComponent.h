

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BoxelPlayerMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BOXEL_API UBoxelPlayerMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UBoxelPlayerMovementComponent();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsAiming() const;

protected:
	virtual void BeginPlay() override;
	
	virtual float GetMaxSpeed() const override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

	UPROPERTY(EditDefaultsOnly)
	float CrouchMovePenaltyMultiplier = 0.4f;
	
	UPROPERTY(EditDefaultsOnly)
	float AimMovePenaltyMultiplier = 0.25f;
	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	

};
