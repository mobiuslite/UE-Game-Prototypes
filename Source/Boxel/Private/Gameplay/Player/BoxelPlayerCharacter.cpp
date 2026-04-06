
#include "Boxel/Public/Gameplay/Player/BoxelPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "Boxel/Public/Gameplay/Player/Movement/BoxelPlayerMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "ToastSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Core/GameHUD.h"
#include "Core/MobiusGameMode.h"
#include "Gameplay/DeathBringer/InventoryComponent.h"
#include "Gameplay/Player/BoxelPlayerState.h"
#include "Gameplay/Weapons/GunBase.h"
#include "Net/UnrealNetwork.h"
#include "Utility/CollisionConsts.h"
#include "Utility/MobiusUtils.h"
#include "Widgets/ToastWidget.h"

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

void ABoxelPlayerCharacter::Client_OnDamageTaken_Implementation(const AController* DamageInstigator, const AActor* DamageCauser,
	const bool bIsDead)
{
	Super::Client_OnDamageTaken_Implementation(DamageInstigator, DamageCauser, bIsDead);
}

void ABoxelPlayerCharacter::Server_OnPlayerDead(const FHitResult& Hit)
{
	AMobiusGameMode* GameMode = GetWorld()->GetAuthGameMode<AMobiusGameMode>();
	if (!GameMode) return;
	
	DeadPlayerInfo.KillshotInfo = Hit;
	if (const ABoxelPlayerState* BoxelPlayerState = GetPlayerState<ABoxelPlayerState>())
	{
		DeadPlayerInfo.TeamId = BoxelPlayerState->GetGenericTeamId();
		DeadPlayerInfo.PlayerName = BoxelPlayerState->GetPlayerName();
	}
	
	GameMode->KillPlayer(this);
	DropHeldGun(false);
	
	EnableRagdoll(Hit);
}

void ABoxelPlayerCharacter::EnableRagdoll_Implementation(const FHitResult& Hit)
{
	if (bIsRagdoll) return;
	bIsRagdoll = true;
	
	USkeletalMeshComponent* RagdollMesh = GetMesh();
	if (!RagdollMesh) return;
	
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	
	const FVector ShotDirection = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal();
	
	RagdollMesh->SetSimulatePhysics(true);
	
	const FVector PushbackVelocity = ShotDirection * RagdollPushbackStrength;
	RagdollMesh->SetAllPhysicsLinearVelocity(GetVelocity() + PushbackVelocity);

	const FVector Origin = RagdollMesh->GetComponentLocation();
	const FVector DistanceFromOrigin = Hit.Location - Origin;
	
	float RotationStrength = 0.25f;
	
	if (DistanceFromOrigin.Z > HeadshotHeight)
	{
		RotationStrength = 1.0f;
	}
	else if (DistanceFromOrigin.Z < LegShotHeight)
	{
		RotationStrength = -0.5f;
	}
	
	const FVector RotateVelocity = ShotDirection.Cross(-FVector::UpVector) * RagdollRotationStrength * RotationStrength;
	RagdollMesh->SetAllPhysicsAngularVelocityInRadians(RotateVelocity);
	
	RagdollMesh->SetCollisionProfileName(FName("Ragdoll"));
}

void ABoxelPlayerCharacter::OnRep_IsRagdoll()
{
	EnableRagdoll(FHitResult());
}

bool ABoxelPlayerCharacter::CanInteract_Implementation() const
{
	return GetPlayerState() == nullptr;
}

void ABoxelPlayerCharacter::Interact_Implementation(APawn* Caller)
{
	if (HasAuthority())
	{
		if (ABoxelPlayerCharacter* Player = Cast<ABoxelPlayerCharacter>(Caller))
		{
			Player->Client_ShowDeadPlayerInfo(DeadPlayerInfo);
		}
	}
}

FText ABoxelPlayerCharacter::GetInteractPreviewString_Implementation() const
{
	//TODO: Localize and keybind
	const FString String = TEXT("Press E to investigate body");
	return FText::FromString(String);
}

void ABoxelPlayerCharacter::Server_PlayerInteracted_Implementation(UObject* Interactable)
{
	if (Interactable && Interactable->Implements<UInteractable>())
	{
		if (IInteractable::Execute_CanInteract(Interactable))
		{
			IInteractable::Execute_Interact(Interactable, this);
		}
	}
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
	
	if (const USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (const UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			OriginalAnimInstanceClass = AnimInstance->GetClass();
		}
	}
}

void ABoxelPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (UToastSubsystem* ToastSubsystem = GetWorld()->GetSubsystem<UToastSubsystem>())
	{
		ToastSubsystem->HideToast(PreviewToastId);
		ToastSubsystem->HideToast(InteractToastId);
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
	
	DoPlayerPreviewTrace();
	DoInteractTrace();
}

//Servers on player state ready
void ABoxelPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (NewController && NewController->IsLocalController())
	{	
		if (const APlayerController* PlayerController = Cast<APlayerController>(NewController))
		{
			if (AGameHUD* HUD = PlayerController->GetHUD<AGameHUD>())
			{
				HUD->OnPlayerStateAdded(PlayerController->GetPlayerState<APlayerState>());
			}
		}
	}
}

//Client player state ready
void ABoxelPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	const APlayerController* PlayerController = GetController<APlayerController>();
	if (PlayerController && PlayerController->IsLocalController())
	{
		if (AGameHUD* GameHUD = PlayerController->GetHUD<AGameHUD>())
		{
			GameHUD->OnPlayerStateAdded(GetPlayerState());
		}
	}
}

void ABoxelPlayerCharacter::OnGunOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//TODO: Check is item is small or big, and see if we can add it to the inventory
	if (HeldItem.GetObject()) return;
	if (!HasAuthority()) return;
	
	if (AGunBase* Gun = Cast<AGunBase>(OtherActor))
	{
		if (!Gun->CanBePickedUp(this)) return;
		PickUpGun(Gun);
	}
}

void ABoxelPlayerCharacter::PickUpGun(AGunBase* Gun)
{
	if (!Gun) return;
	if (!HasAuthority()) return;
	
	UInventoryComponent* Inventory;
	if (!UMobiusUtils::GetInventory(GetPlayerState(), Inventory)) return;
	
	Inventory->AddItem(TScriptInterface<IInventoryItem>(Gun));
	
	HeldItem = TScriptInterface<IInventoryItem>(Gun);
	OnRep_HeldItem(nullptr);
}

void ABoxelPlayerCharacter::DropHeldGun_Implementation(const bool bThrow)
{
	if (!HeldItem.GetObject()) return;
	if (!HasAuthority()) return;
	
	const AGunBase* HeldGun = Cast<AGunBase>(HeldItem.GetObject());
	if (!HeldGun || !HeldGun->IsDroppable()) return;
	
	UInventoryComponent* Inventory;
	if (!UMobiusUtils::GetInventory(GetPlayerState(), Inventory)) return;
	
	Inventory->RemoveItem(HeldItem);
	
	const TScriptInterface<IInventoryItem> PreviousGun = HeldItem;
	HeldItem = TScriptInterface<IInventoryItem>(nullptr);
	OnRep_HeldItem(PreviousGun);
}

void ABoxelPlayerCharacter::OnRep_HeldItem(const TScriptInterface<IInventoryItem>& LastGun)
{
	if (HeldItem.GetObject())
	{
		const AGunBase* HeldGun = Cast<AGunBase>(HeldItem.GetObject());
		
		
		if (const TSubclassOf<UAnimInstance> GunABP = HeldGun->GetGunAnimInstanceClass())
		{
			GetMesh()->SetAnimInstanceClass(GunABP);
		}
		
		if (const ABoxelPlayerState* BoxelState = GetPlayerState<ABoxelPlayerState>())
		{
			BoxelState->BroadcastGunEquipped(HeldGun);
		}
	}
	
	if (LastGun.GetObject())
	{
		const AGunBase* LastGunBase = Cast<AGunBase>(LastGun.GetObject());
		
		GetMesh()->SetAnimInstanceClass(OriginalAnimInstanceClass);
		
		if (const ABoxelPlayerState* BoxelState = GetPlayerState<ABoxelPlayerState>())
		{
			BoxelState->BroadcastGunUnequipped(LastGunBase);
		}
	}
}

void ABoxelPlayerCharacter::DoPlayerPreviewTrace()
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}
	
	FVector StartLocation;
	FVector EndLocation;
	UMobiusUtils::GetCameraControlTraceLocation(PlayerController, PreviewPlayerInfoDistance, StartLocation, EndLocation);
		
	const FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(PlayerPreviewTrace), false, this);
		
	//Show widget that shows player's name and approximate HP
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WeaponTrace, TraceParams))
	{
		if (PreviewToastId != -1 || !IsValid(PreviewPlayerToastClass)) return;
		
		const APawn* HitPawn = Cast<APawn>(HitResult.GetActor());
		if (!HitPawn) return;
		
		const APlayerState* HitPlayerState = HitPawn->GetPlayerState<APlayerState>();
		if (!HitPlayerState) return;
		
		if (UToastSubsystem* ToastSubsystem = GetWorld()->GetSubsystem<UToastSubsystem>())
		{
			ToastSubsystem->ShowManualToast(PreviewPlayerToastClass, HitPlayerState->GetPlayerName(), 
				FVector2D(0.0f, 50.0f), FAnchors(0.5f), FVector2D(0.5f), PreviewToastId);
		}
	}
	else
	{
		if (PreviewToastId == -1) return;
		
		if (UToastSubsystem* ToastSubsystem = GetWorld()->GetSubsystem<UToastSubsystem>())
		{
			ToastSubsystem->HideToast(PreviewToastId);
			PreviewToastId = -1;
		}
	}
}

void ABoxelPlayerCharacter::Client_ShowDeadPlayerInfo_Implementation(const FDeadPlayerInfo& Info)
{
	BP_ShowDeadPlayerInfo(Info);
}

void ABoxelPlayerCharacter::DoInteractTrace()
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}
	
	FVector StartLocation;
	FVector EndLocation;
	UMobiusUtils::GetCameraControlTraceLocation(PlayerController, PlayerInteractDistance, StartLocation, EndLocation);
		
	const FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(InteractTrace), false, this);
	
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_InteractableTrace, TraceParams))
	{
		if (!HitResult.GetActor()) return;
		if (!HitResult.GetActor()->Implements<UInteractable>()) return;
		if (CurrentInteractable.GetObject() == HitResult.GetActor()) return;
		
		const TScriptInterface<IInteractable> Interactable = TScriptInterface<IInteractable>(HitResult.GetActor());
		if (!IInteractable::Execute_CanInteract(Interactable.GetObject())) return;
		
		CurrentInteractable = Interactable;
		
		if (!IsValid(PreviewPlayerToastClass)) return;
		
		if (UToastSubsystem* ToastSubsystem = GetWorld()->GetSubsystem<UToastSubsystem>())
		{
			if (InteractToastId != -1)
			{
				ToastSubsystem->HideToast(InteractToastId);
			}
				
			ToastSubsystem->ShowManualToast(InteractToastClass, 
				IInteractable::Execute_GetInteractPreviewString(Interactable.GetObject()).ToString(), 
				FVector2D(0.0f, 100.0f), FAnchors(0.5f), FVector2D(0.5f), InteractToastId);
		}
	}
	else
	{
		if (IsValid(CurrentInteractable.GetObject()))
		{
			CurrentInteractable = TScriptInterface<IInteractable>(nullptr);
		}
		
		if (InteractToastId != -1)
		{
			if (UToastSubsystem* ToastSubsystem = GetWorld()->GetSubsystem<UToastSubsystem>())
			{
				ToastSubsystem->HideToast(InteractToastId);
				InteractToastId = -1;
			}
		}
	}
}

void ABoxelPlayerCharacter::OnTriggerReleased_Implementation()
{
	if (UAbilitySystemComponent* AbilityComp = GetAbilitySystemComponent())
	{
		AbilityComp->ReleaseInputID(1);
	} 
}

void ABoxelPlayerCharacter::OnTriggerPressed_Implementation()
{
	if (UAbilitySystemComponent* AbilityComp = GetAbilitySystemComponent())
	{
		AbilityComp->PressInputID(1);
	} 
	
	if (AGunBase* HeldGun = Cast<AGunBase>(HeldItem.GetObject()))
	{
		if (HeldGun && HeldGun->GetAmmoCount() == 0)
		{
			HeldGun->StartReload();
		}
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

void ABoxelPlayerCharacter::DropInput(const FInputActionValue& Value)
{
	DropHeldGun();

	if (!HasAuthority())
	{
		//Client prediction
		if (AGunBase* HeldGun = Cast<AGunBase>(HeldItem.GetObject()))
		{
			if (HeldGun)
			{
				UInventoryComponent* Inventory;
				if (!UMobiusUtils::GetInventory(GetPlayerState(), Inventory)) return;
	
				Inventory->RemoveItem(TScriptInterface<IInventoryItem>(HeldGun));
			}
		}
	}
}

void ABoxelPlayerCharacter::InteractInput(const FInputActionValue& Value)
{
	if (UObject* Interactable = CurrentInteractable.GetObject())
	{
		IInteractable::Execute_Interact(Interactable, this);
		if (!HasAuthority())
		{
			Server_PlayerInteracted(Interactable);
		}
	}
}

void ABoxelPlayerCharacter::ReloadInput(const FInputActionValue& Value)
{
	if (AGunBase* HeldGun = Cast<AGunBase>(HeldItem.GetObject()))
	{
		if (HeldGun)
		{
			HeldGun->StartReload();
		}
	}
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
		
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::DropInput);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::InteractInput);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::ReloadInput);
		
		EnhancedInputComponent->BindAction(PushToTalkAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::TalkInput);
		EnhancedInputComponent->BindAction(PushToTalkAction, ETriggerEvent::Completed, this, &ABoxelPlayerCharacter::TalkInput_Released);
		
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::PauseInput);
	}
}

void ABoxelPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, bTalking);
	DOREPLIFETIME(ThisClass, HeldItem);
}

