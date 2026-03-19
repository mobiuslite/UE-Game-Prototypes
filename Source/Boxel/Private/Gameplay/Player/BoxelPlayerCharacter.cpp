
#include "Boxel/Public/Gameplay/Player/BoxelPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "Boxel/Public/Gameplay/Player/Movement/BoxelPlayerMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Core/GameHUD.h"
#include "Gameplay/Player/BoxelPlayerState.h"
#include "Gameplay/Weapons/Projectiles/Projectile.h"
#include "MobiusAbilitySystem/Attributes/MACommonAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Utility/MobiusUtils.h"

ABoxelPlayerCharacter::ABoxelPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBoxelPlayerMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
}

FRotator ABoxelPlayerCharacter::GetViewRotation() const
{
	FRotator Result = GetActorRotation();
	
	if (GetController() != nullptr)
	{
		Result = GetController()->GetControlRotation();
	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		// check if being spectated
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			const APlayerController* PlayerController = Iterator->Get();
			if (PlayerController &&
				PlayerController->PlayerCameraManager &&
				PlayerController->PlayerCameraManager->GetViewTargetPawn() == this)
			{
				Result = PlayerController->BlendedTargetViewRotation;
			}
		}
	}

	return Result + ExtraViewRotation;
}

void ABoxelPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABoxelPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (FireTimer > 0.0f)
	{
		FireTimer -= DeltaTime;
	}
	
	if (IsValid(ProjectileClass) && bTriggerDown && FireTimer <= 0.0f)
	{
		const auto ASC = GetAbilitySystemComponent();
		
		bool bFound;
		const float AttackSpeed = ASC->GetGameplayAttributeValue(UMACommonAttributeSet::GetAttackSpeedAttribute(), bFound);
		
		const float SecondsBetweenFire = (1.0f / AttackSpeed) * 60.0f;
		FireTimer += SecondsBetweenFire;
		
		FVector Location;
		UMobiusUtils::GetCameraLocation(GetController(), Location);
	
		Location += GetControlRotation().Vector() * 125.0f;
	
		Server_FireProjectile(Location, GetControlRotation());
	
		if (!HasAuthority())
		{
			FireProjectile(Location, GetControlRotation());
		}
	}
	
	if (const UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		if (MovementComp->MovementMode == MOVE_Falling)
		{
			TimeSpentInAir += DeltaTime;
		}
	}
}

void ABoxelPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void ABoxelPlayerCharacter::OnTriggerReleased_Implementation()
{
	bTriggerDown = false;
}

void ABoxelPlayerCharacter::OnTriggerPressed_Implementation()
{
	bTriggerDown = true;
}

void ABoxelPlayerCharacter::Server_FireProjectile_Implementation(const FVector& Location, const FRotator& Rotation)
{
	FireProjectile(Location, Rotation);
}

void ABoxelPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	OnLandedEX(TimeSpentInAir, Hit);
	TimeSpentInAir = 0.0f;
}

void ABoxelPlayerCharacter::OnLandedEX_Implementation(const float TimeInAir, const FHitResult& Hit)
{
}

void ABoxelPlayerCharacter::OnRep_Talking_Implementation()
{
	if (ABoxelPlayerState* BoxelPlayerState = GetPlayerState<ABoxelPlayerState>())
	{
		BoxelPlayerState->SetSpeaking(bTalking);
	}
}

void ABoxelPlayerCharacter::FireProjectile(const FVector& Location, const FRotator& Rotation)
{
	//const FTransform ProjectileTransform = FTransform(Rotation, Location);

	//AProjectile* NewProjectile = Cast<AProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ProjectileClass, ProjectileTransform,
	//	ESpawnActorCollisionHandlingMethod::AlwaysSpawn, this));
	//
	//UGameplayStatics::FinishSpawningActor(NewProjectile, ProjectileTransform, ESpawnActorScaleMethod::MultiplyWithRoot);
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	GetWorld()->SpawnActor<AProjectile>(ProjectileClass, Location, Rotation, SpawnParams);
}

void ABoxelPlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	
	const FRotator ControlRotation = GetControlRotation();
	const FVector ForwardDirection = GetActorForwardVector();
	const FVector RightDirection = FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::Y);
	
	AddMovementInput(ForwardDirection, Input.Y);
	AddMovementInput(RightDirection, Input.X);
}

void ABoxelPlayerCharacter::LookInput(const FInputActionValue& Value)
{
	const FVector2D MouseInput = Value.Get<FVector2D>();
	AddControllerYawInput(MouseInput.X);
	AddControllerPitchInput(-MouseInput.Y);
}

void ABoxelPlayerCharacter::JumpInput(const FInputActionValue& Value)
{
	Jump();
}

void ABoxelPlayerCharacter::JumpInput_Released(const FInputActionValue& Value)
{
	StopJumping();
}

void ABoxelPlayerCharacter::FireInput(const FInputActionValue& Value)
{
	OnTriggerPressed();
}

void ABoxelPlayerCharacter::FireInput_Released(const FInputActionValue& Value)
{
	OnTriggerReleased();
}

void ABoxelPlayerCharacter::TalkInput(const FInputActionValue& Value)
{
	if (!bTalking)
	{
		bTalking = true;
		OnRep_Talking();
	}
}

void ABoxelPlayerCharacter::TalkInput_Released(const FInputActionValue& Value)
{
	if (bTalking)
	{
		bTalking = false;
		OnRep_Talking();
	}
}

void ABoxelPlayerCharacter::PauseInput(const FInputActionValue& Value)
{
	if (const APlayerController* PlayerController = GetController<APlayerController>())
	{
		if (AGameHUD* GameHUD = PlayerController->GetHUD<AGameHUD>())
		{
			GameHUD->PauseGame();
		}
	}
}

void ABoxelPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABoxelPlayerCharacter::MoveInput);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABoxelPlayerCharacter::LookInput);
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::JumpInput);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABoxelPlayerCharacter::JumpInput_Released);
		
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::FireInput);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABoxelPlayerCharacter::FireInput_Released);
		
		EnhancedInputComponent->BindAction(PushToTalkAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::TalkInput);
		EnhancedInputComponent->BindAction(PushToTalkAction, ETriggerEvent::Completed, this, &ABoxelPlayerCharacter::TalkInput_Released);
		
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::PauseInput);
	}
}

void ABoxelPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, bTalking);
}

