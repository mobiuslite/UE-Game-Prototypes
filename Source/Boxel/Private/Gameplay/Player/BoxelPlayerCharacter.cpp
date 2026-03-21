
#include "Boxel/Public/Gameplay/Player/BoxelPlayerCharacter.h"
#include "Boxel/Public/Gameplay/Player/Movement/BoxelPlayerMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Components/CapsuleComponent.h"
#include "Core/GameHUD.h"
#include "Gameplay/Player/BoxelPlayerState.h"
#include "Gameplay/Weapons/GunBase.h"
#include "Net/UnrealNetwork.h"
#include "Utility/CollisionConsts.h"

ABoxelPlayerCharacter::ABoxelPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBoxelPlayerMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	
	Capsule->SetCollisionResponseToChannel(ECC_Gun, ECR_Overlap);
	Capsule->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::ABoxelPlayerCharacter::OnGunOverlap);
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
	
	if (HasAuthority() && IsValid(StartingGunClass))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		AGunBase* NewGun = GetWorld()->SpawnActor<AGunBase>(StartingGunClass, SpawnParams);
		PickUpGun(NewGun);
	}
}

void ABoxelPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

void ABoxelPlayerCharacter::OnGunOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HeldGun) return;
	if (!HasAuthority()) return;
	
	if (AGunBase* Gun = Cast<AGunBase>(OtherActor))
	{
		PickUpGun(Gun);
	}
}

void ABoxelPlayerCharacter::PickUpGun(AGunBase* Gun)
{
	if (!Gun) return;
	if (!HasAuthority()) return;
	
	HeldGun = Gun;
	OnRep_HeldGun(nullptr);
}

void ABoxelPlayerCharacter::DropHeldGun()
{
	if (!HeldGun) return;
	if (!HeldGun->IsDroppable()) return;
	
	HeldGun->SetHolder(nullptr);
	
	AGunBase* PreviousGun = HeldGun;
	HeldGun = nullptr;
	OnRep_HeldGun(PreviousGun);
}

void ABoxelPlayerCharacter::OnRep_HeldGun(AGunBase* LastGun)
{
	if (HeldGun)
	{
		HeldGun->SetHolder(this);
	}
	
	if (LastGun)
	{
		LastGun->SetHolder(nullptr);
	}
}

void ABoxelPlayerCharacter::OnTriggerReleased_Implementation()
{
	if (HeldGun)
	{
		HeldGun->ReleaseTrigger();
	}
}

void ABoxelPlayerCharacter::OnTriggerPressed_Implementation()
{
	if (HeldGun)
	{
		HeldGun->PullTrigger();
	}
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
	DOREPLIFETIME(ThisClass, HeldGun);
}

