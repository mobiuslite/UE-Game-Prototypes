

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

protected:
	virtual void BeginPlay() override;
	
	virtual float GetMaxSpeed() const override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
