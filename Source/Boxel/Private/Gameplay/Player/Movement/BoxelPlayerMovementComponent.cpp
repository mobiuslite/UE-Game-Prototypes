#include "Boxel/Public/Gameplay/Player/Movement/BoxelPlayerMovementComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "MobiusAbilitySystem/Attributes/MACommonAttributeSet.h"

UBoxelPlayerMovementComponent::UBoxelPlayerMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bUseFlatBaseForFloorChecks = true;
}

void UBoxelPlayerMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

float UBoxelPlayerMovementComponent::GetMaxSpeed() const
{
	bool bFoundMoveSpeed = false;
	float MoveSpeed = 0.0f;
	if (const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawnOwner()))
	{
		MoveSpeed = ASC->GetGameplayAttributeValue(UMACommonAttributeSet::GetMoveSpeedAttribute(), bFoundMoveSpeed);
	}
	
	switch(MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		if (bFoundMoveSpeed) return MoveSpeed;
		
		return IsCrouching() ? MaxWalkSpeedCrouched : MaxWalkSpeed;
	case MOVE_Falling:
		if (bFoundMoveSpeed) return MoveSpeed;
		
		return MaxWalkSpeed;
	case MOVE_Swimming:
		return MaxSwimSpeed;
	case MOVE_Flying:
		return MaxFlySpeed;
	case MOVE_Custom:
		return MaxCustomMovementSpeed;
	case MOVE_None:
	default:
		return 0.f;
	}
}

void UBoxelPlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

