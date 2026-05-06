
#include "Boxel/Public/Gameplay/Player/BoxelPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Boxel/Public/Gameplay/Player/Movement/BoxelPlayerMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "ToastSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/GameHUD.h"
#include "Core/MobiusGameMode.h"
#include "Core/DeathBringer/DeathBringerGameMode.h"
#include "Core/Lobby/LobbyGameState.h"
#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"
#include "Gameplay/Player/BoxelPlayerState.h"
#include "Gameplay/Weapons/GunBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "MobiusAbilitySystem/Attributes/MACommonAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "SaveSystem/BoxelSaveSubsystem.h"
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

	return Result + ExtraViewRotation + FRotator(CurrentRecoilAmount, 0.0f, 0.0f);
}

void ABoxelPlayerCharacter::Client_OnDamageTaken_Implementation(const AController* DamageInstigator, const AActor* DamageCauser,
                                                                const bool bIsDead)
{
	if (bIsDead)
	{
		SetSpeaking(false, false);
		OnAimReleased();
		CleanupToasts();
	}
	
	BP_LocalOnTakeDamage(DamageInstigator, DamageCauser, bIsDead);
}

void ABoxelPlayerCharacter::Server_OnPlayerDead(const FHitResult& Hit, const AActor* Causer)
{
	AMobiusGameMode* GameMode = GetWorld()->GetAuthGameMode<AMobiusGameMode>();
	if (!GameMode) return;
	
	DropAllItems();
	
	DeadPlayerInfo.KillshotInfo = Hit;
	DeadPlayerInfo.KilledByWeaponName = Causer ? Causer->GetName() : "UNKNOWN";
	
	if (const ABoxelPlayerState* BoxelPlayerState = GetPlayerState<ABoxelPlayerState>())
	{
		DeadPlayerInfo.TeamId = BoxelPlayerState->GetGenericTeamId();
		DeadPlayerInfo.PlayerName = BoxelPlayerState->GetPlayerName();
	}
	
	GameMode->KillPlayer(this);
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
	
	RagdollMesh->SetSimulatePhysics(true);
	
	if (Hit.HasValidHitObjectHandle())
	{
		const FVector ShotDirection = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal();
	
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
	}
	
	RagdollMesh->SetCollisionProfileName(FName("Ragdoll"));
}

void ABoxelPlayerCharacter::OnRep_IsRagdoll()
{
	EnableRagdoll(FHitResult());
}

bool ABoxelPlayerCharacter::CanInteract_Implementation() const
{
	if (GetWorld()->GetGameState<ALobbyGameState>())
	{
		return false;
	}
	
	return GetPlayerState() == nullptr;
}

void ABoxelPlayerCharacter::Interact_Implementation(APawn* Caller)
{
	if (HasAuthority())
	{
		if (ABoxelPlayerCharacter* Player = Cast<ABoxelPlayerCharacter>(Caller))
		{
			FDeadPlayerInfo PlayerInfo = DeadPlayerInfo;
			
			//Only saviours get full death info
			if (UMobiusUtils::GetTeamId(Caller)!= EDeathBringerTeam::Saviour)
			{
				PlayerInfo.KillshotInfo = FHitResult();
				PlayerInfo.KilledByWeaponName = "";
			}
			
			Player->Client_ShowDeadPlayerInfo(PlayerInfo);
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

void ABoxelPlayerCharacter::AddFOVEffect(const float Strength, const float Duration, const bool bInstantSet)
{
	FCameraFOVEffect Effect;
	Effect.Strength = Strength;
	Effect.Timer = Duration;
	
	FOVEffects.Add(Effect);
	
	if (bInstantSet)
	{
		CurrentCameraFOV += Strength;
	}
}

void ABoxelPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		if (IsValid(StartingGunClass))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.bNoFail = true;
			PickUpItem(GetWorld()->SpawnActor<AInventoryItem>(StartingGunClass, SpawnParams));
		}
	}
	
	if (const USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (const UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			OriginalAnimInstanceClass = AnimInstance->GetClass();
		}
	}
	
	PlayerCamera = GetComponentByClass<UCameraComponent>();
	if (PlayerCamera)
	{
		if (UBoxelSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UBoxelSaveSubsystem>())
		{
			PlayerCamera->SetFieldOfView(SaveSubsystem->GetFOV());
			CurrentCameraFOV = SaveSubsystem->GetFOV();
		}
	}
}

void ABoxelPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	CleanupToasts();
}

void ABoxelPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	DoPlayerPreviewTrace();
	DoInteractTrace();
	
	if (PlayerCamera && IsLocallyControlled())
	{
		float BaseFOV = 90.0f;
		if (UBoxelSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UBoxelSaveSubsystem>())
		{
			BaseFOV = SaveSubsystem->GetFOV();
		}
		
		float FOVToAdd = 0.0f;
		for (int i = 0; i < FOVEffects.Num();)
		{
			FCameraFOVEffect& Effect = FOVEffects[i];
			Effect.Timer -= DeltaSeconds;
			if (Effect.Timer <= 0.0f)
			{
				FOVEffects.RemoveAt(i);
			}
			else
			{
				i++;
				FOVToAdd += Effect.Strength;
			}
		}
		
		CurrentCameraFOV = UMobiusUtils::StableLerpFloat(CurrentCameraFOV, BaseFOV + FOVToAdd, FOVChangeSpeed, DeltaSeconds);
		PlayerCamera->SetFieldOfView(CurrentCameraFOV / CurrentZoomAmount);
		
		if (!FMath::IsNearlyZero(RecoilTarget))
		{
			RecoilRecoverTimer -= DeltaSeconds;
			if (RecoilRecoverTimer <= 0.0f)
			{
				RecoilTarget = 0.0f;
			}
			else
			{
				RecoilTarget = FMath::Clamp(UKismetMathLibrary::Ease(RecoilTarget, 0.0f, 1.0f - (RecoilRecoverTimer / RecoilRecoverDuration), EEasingFunc::ExpoIn), 0.0f, FLT_MAX);
				if (RecoilTarget < 0.0f)
				{
					RecoilTarget = 0.0f;
				}
			}
		
			CurrentRecoilAmount = UMobiusUtils::StableLerpFloat(CurrentRecoilAmount, RecoilTarget, RecoilLerpSpeed, DeltaSeconds);
		}
	}
}

//Servers on player state ready
void ABoxelPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (NewController)
	{	
		if (NewController->IsLocalController())
		{
			if (const APlayerController* PlayerController = Cast<APlayerController>(NewController))
			{
				if (AGameHUD* HUD = PlayerController->GetHUD<AGameHUD>())
				{
					HUD->OnPlayerStateAdded(PlayerController->GetPlayerState<APlayerState>());
				}
			}
			
			OnLocalPlayerStateReady(GetPlayerState<ABoxelPlayerState>());
		}
		else
		{
			OnProxyPlayerStateReady(GetPlayerState<ABoxelPlayerState>());
		}
	}
	
	if (!HeldItem && IsValid(UnarmedAbilityClass) && !UnarmedAbilityHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			UnarmedAbilityHandle = ASC->K2_GiveAbility(UnarmedAbilityClass, 0, 1);
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
		
		OnLocalPlayerStateReady(GetPlayerState<ABoxelPlayerState>());
	}
	else
	{
		OnProxyPlayerStateReady(GetPlayerState<ABoxelPlayerState>());
	}
}

void ABoxelPlayerCharacter::OnLocalPlayerStateReady_Implementation(ABoxelPlayerState* LocalPlayerState)
{
	if (LocalPlayerState) LocalPlayerState->OnLocalPlayerStateReady();
}

void ABoxelPlayerCharacter::OnProxyPlayerStateReady_Implementation(ABoxelPlayerState* ProxyPlayerState)
{
	if (ProxyPlayerState) ProxyPlayerState->OnProxyPlayerStateReady();
}

void ABoxelPlayerCharacter::OnAimPressed_Implementation()
{
	if (AGunBase* Gun = Cast<AGunBase>(HeldItem))
	{
		Gun->GetGunZoomAmount(CurrentZoomAmount);
		
		if (IsValid(Gun->GetAimWidgetClass()))
		{
			if (UToastSubsystem* ToastSubsystem = GetWorld()->GetSubsystem<UToastSubsystem>())
			{
				ToastSubsystem->ShowManualToast(Gun->GetAimWidgetClass(), 
					TEXT(""), FVector2D(0.0f), FAnchors(0.0f, 0.0f, 1.0f, 1.0f), FVector2D(0.5f), AimToastId);
			}
		}
	}
}

void ABoxelPlayerCharacter::OnAimReleased_Implementation()
{
	float BaseFOV = 90.0f;
	if (UBoxelSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UBoxelSaveSubsystem>())
	{
		BaseFOV = SaveSubsystem->GetFOV();
	}
	
	CurrentZoomAmount = 1.0f;
	
	if (UCameraComponent* Camera = Cast<UCameraComponent>(GetComponentByClass(UCameraComponent::StaticClass())))
	{
		Camera->SetFieldOfView(BaseFOV);
	}
	
	if (AimToastId != INDEX_NONE)
	{
		if (UToastSubsystem* ToastSubsystem = GetWorld()->GetSubsystem<UToastSubsystem>())
		{
			ToastSubsystem->HideToast(AimToastId);
			AimToastId = INDEX_NONE;
		}
	}
}

void ABoxelPlayerCharacter::OnGunOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	
	if (OtherActor->IsA(AInventoryItem::StaticClass()))
	{
		AInventoryItem* Item = Cast<AInventoryItem>(OtherActor);
		ensure(Item);
		
		if (!Item->CanBePickedUp(this)) return;
		PickUpItem(Item);
	}
}

bool ABoxelPlayerCharacter::PickUpItem(AInventoryItem* Item, const bool bConceal)
{
	if (!Item) return false;
	if (!HasAuthority()) return false;
	
	UInventoryComponent* Inventory;
	if (!UMobiusUtils::GetInventory(GetPlayerState(), Inventory)) return false;
	
	const bool bSuccess = Inventory->AddItem(Item);
	if (bSuccess && Inventory->GetItemCount() == 1 && !bConceal)
	{
		Item->OnEquip(GetController());
		HeldItem = Item;
		OnRep_HeldItem(nullptr);
	}
	
	return bSuccess;
}

void ABoxelPlayerCharacter::DropAllItems()
{
	if (!HasAuthority()) return;
	
	const AInventoryItem* PreviousGun = HeldItem;
	HeldItem = nullptr;
	
	UInventoryComponent* Inventory;
	if (UMobiusUtils::GetInventory(GetPlayerState(), Inventory))
	{
		while (Inventory->GetItemCount() != 0)
		{
			AInventoryItem* Item = Inventory->GetItemByIndex(0);
			Inventory->RemoveItem(Item);
		}
	}
	
	OnRep_HeldItem(PreviousGun);
}

void ABoxelPlayerCharacter::AddRecoil(const float Amount, const float MaxRecoilAmount)
{
	ensure (MaxRecoilAmount > 0.0f);
	
	const float Multiplier = UKismetMathLibrary::Ease(1.0f, 0.0f, RecoilTarget / MaxRecoilAmount, EEasingFunc::EaseOut);
	RecoilTarget += Amount * Multiplier;
	
	RecoilRecoverTimer = RecoilRecoverDuration;
}

void ABoxelPlayerCharacter::DropHeldGun_Implementation(const bool bThrow)
{
	if (!HeldItem) return;
	if (!HasAuthority()) return;
	if (!HeldItem->CanBeDropped()) return;
	
	AInventoryItem* PreviousItem = HeldItem;
	HeldItem = nullptr;
	
	PreviousItem->OnUnequip(GetController());
	
	UInventoryComponent* Inventory;
	if (UMobiusUtils::GetInventory(GetPlayerState(), Inventory))
	{
		Inventory->RemoveItem(PreviousItem);
	}
	
	OnRep_HeldItem(PreviousItem);
}

void ABoxelPlayerCharacter::OnRep_HeldItem(const AInventoryItem* LastGun)
{
	if (HeldItem)
	{
		if (HasAuthority() && UnarmedAbilityHandle.IsValid())
		{
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				ASC->ClearAbility(UnarmedAbilityHandle);
				UnarmedAbilityHandle = FGameplayAbilitySpecHandle();
			}
		}
		
		if (const TSubclassOf<UAnimInstance> GunABP = HeldItem->GetHeldABPClass())
		{
			GetMesh()->SetAnimInstanceClass(GunABP);
		}
		
		if (const AGunBase* HeldGun = Cast<AGunBase>(HeldItem))
		{
			if (const ABoxelPlayerState* BoxelState = GetPlayerState<ABoxelPlayerState>())
			{
				BoxelState->BroadcastGunEquipped(HeldGun);
			}
		}
	}
	else
	{
		if (HasAuthority() && IsValid(UnarmedAbilityClass))
		{
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				UnarmedAbilityHandle = ASC->K2_GiveAbility(UnarmedAbilityClass, 0, 1);
			}
		}
		
		const AGunBase* LastGunBase = nullptr;
		if (LastGun)
		{
			LastGunBase = Cast<AGunBase>(LastGun);
		}
		
		GetMesh()->SetAnimInstanceClass(OriginalAnimInstanceClass);
		
		if (const ABoxelPlayerState* BoxelState = GetPlayerState<ABoxelPlayerState>())
		{
			BoxelState->BroadcastGunUnequipped(LastGunBase);
		}
	}
}

void ABoxelPlayerCharacter::SetHeldItemIndex(int Index)
{
	UInventoryComponent* Inventory;
	if (UMobiusUtils::GetInventory(GetPlayerState(), Inventory))
	{
		if (Inventory->GetItemCount() == 0) return;
		
		if (Index < -1)
		{
			Index = Inventory->GetItemCount() - 1;
		}
		else if (Index >= Inventory->GetItemCount())
		{
			Index = -1;
		}
		
		AInventoryItem* OldItem = HeldItem;
		HeldItem = Inventory->GetItemByIndex(Index);
		
		if (OldItem)
		{
			OldItem->OnUnequip(GetController());
		}
		
		if (HeldItem)
		{
			HeldItem->OnEquip(GetController());
		}
		
		OnRep_HeldItem(OldItem);
		
		if (!HasAuthority())
		{
			Server_SetHeldItemIndex(Index);
		}
	}
}

void ABoxelPlayerCharacter::Server_SetHeldItemIndex_Implementation(const int Index)
{
	SetHeldItemIndex(Index);
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
		
		const ACharacter* HitCharacter = Cast<ACharacter>(HitResult.GetActor());
		if (!HitCharacter) return;
		
		const APlayerState* HitPlayerState = HitCharacter->GetPlayerState<APlayerState>();
		if (!HitPlayerState) return;
		
		if (UToastSubsystem* ToastSubsystem = GetWorld()->GetSubsystem<UToastSubsystem>())
		{
			int PlayerHealthPercent = 100;
			if (const UAbilitySystemComponent* HitAbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitPlayerState))
			{
				const float CurrentHealth = HitAbilityComponent->GetNumericAttribute(UMACommonAttributeSet::GetCurrentHealthAttribute());
				const float MaxHealth = HitAbilityComponent->GetNumericAttribute(UMACommonAttributeSet::GetMaxHealthAttribute());
				
				PlayerHealthPercent = FMath::RoundToInt(CurrentHealth / MaxHealth * 100);
			}
			
			const FString PlayerToastMsg = FString::Printf(TEXT("%s:%d"), *HitPlayerState->GetPlayerName(), PlayerHealthPercent);
			
			ToastSubsystem->ShowManualToast(PreviewPlayerToastClass, PlayerToastMsg, 
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
	
	if (AGunBase* HeldGun = Cast<AGunBase>(HeldItem))
	{
		if (HeldGun && HeldGun->GetAmmoCount() == 0)
		{
			HeldGun->StartReload();
		}
	}
}

void ABoxelPlayerCharacter::Landed(const FHitResult& Hit)
{
	ensure(GetCharacterMovement());
	OnLandedEX(GetCharacterMovement()->Velocity.Z, Hit);
	
	Super::Landed(Hit);
}

void ABoxelPlayerCharacter::SetPlayerUnarmed()
{
	AInventoryItem* PreviousItem = HeldItem;
	HeldItem = nullptr;
	
	if (PreviousItem)
	{
		PreviousItem->OnUnequip(GetController());
	}
	
	OnRep_HeldItem(PreviousItem);
}

void ABoxelPlayerCharacter::OnLandedEX_Implementation(const float ZVelocity, const FHitResult& Hit)
{
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
	float MouseSens = 1.0f;
	if (UBoxelSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UBoxelSaveSubsystem>())
	{
		MouseSens = SaveSubsystem->GetMouseSensitivity();
	}
	
	if (CurrentZoomAmount > 0.0f)
	{
		MouseSens /= CurrentZoomAmount; 
	}
	
	const FVector2D MouseInput = Value.Get<FVector2D>() * MouseSens;
	
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

void ABoxelPlayerCharacter::AimInput(const FInputActionValue& Value)
{
	OnAimPressed();
}

void ABoxelPlayerCharacter::AimInput_Released(const FInputActionValue& Value)
{
	OnAimReleased();
}

void ABoxelPlayerCharacter::DropInput(const FInputActionValue& Value)
{
	if (!HeldItem) return;
	if (!HeldItem->CanBeDropped()) return;
	
	OnAimReleased();
	DropHeldGun();

	if (!HasAuthority())
	{
		//Client prediction
		if (HeldItem)
		{
			UInventoryComponent* Inventory;
			if (!UMobiusUtils::GetInventory(GetPlayerState(), Inventory)) return;
	
			Inventory->RemoveItem(HeldItem);
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
	if (AGunBase* HeldGun = Cast<AGunBase>(HeldItem))
	{
		if (HeldGun)
		{
			HeldGun->StartReload();
		}
	}
}

void ABoxelPlayerCharacter::ScrollInventoryInput(const FInputActionValue& Value)
{
	const bool bScrollDirection = Value.Get<float>() > 0.0f;
	
	UInventoryComponent* Inventory;
	if (UMobiusUtils::GetInventory(GetPlayerState(), Inventory))
	{
		const int CurrentIndex = Inventory->GetIndexOfItem(HeldItem);
		SetHeldItemIndex(CurrentIndex + (bScrollDirection ? 1 : -1));
	}
	
	OnAimReleased();
}

void ABoxelPlayerCharacter::TalkInput(const FInputActionValue& Value)
{
	if (!bRadioTalking)
	{
		bRadioTalking = true;
		SetSpeaking(bRadioTalking, false);
	}
}

void ABoxelPlayerCharacter::TalkInput_Released(const FInputActionValue& Value)
{
	if (bRadioTalking)
	{
		bRadioTalking = false;
		SetSpeaking(bRadioTalking, false);
	}
}

void ABoxelPlayerCharacter::TeamTalkInput(const FInputActionValue& Value)
{
	if (!bTeamTalking)
	{
		bTeamTalking = true;
		SetSpeaking(bTeamTalking, true);
	}
}

void ABoxelPlayerCharacter::TeamTalkInput_Released(const FInputActionValue& Value)
{
	if (bTeamTalking)
	{
		bTeamTalking = false;
		SetSpeaking(bTeamTalking, true);
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

void ABoxelPlayerCharacter::CleanupToasts()
{
	if (UToastSubsystem* ToastSubsystem = GetWorld()->GetSubsystem<UToastSubsystem>())
	{
		ToastSubsystem->HideToast(PreviewToastId);
		ToastSubsystem->HideToast(InteractToastId);
		ToastSubsystem->HideToast(AimToastId);
	}
}

void ABoxelPlayerCharacter::SetSpeaking(const bool bSpeaking, const bool bTeamSpeaking)
{
	if (ABoxelPlayerState* BoxelPlayerState = GetPlayerState<ABoxelPlayerState>())
	{
		BoxelPlayerState->SetSpeaking(bSpeaking, bTeamSpeaking);
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
		
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::AimInput);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABoxelPlayerCharacter::AimInput_Released);
		
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::DropInput);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::InteractInput);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::ReloadInput);
		
		EnhancedInputComponent->BindAction(ScrollInventoryAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::ScrollInventoryInput);
		
		EnhancedInputComponent->BindAction(PushToTalkAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::TalkInput);
		EnhancedInputComponent->BindAction(PushToTalkAction, ETriggerEvent::Completed, this, &ABoxelPlayerCharacter::TalkInput_Released);
		
		EnhancedInputComponent->BindAction(TeamTalkAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::TeamTalkInput);
		EnhancedInputComponent->BindAction(TeamTalkAction, ETriggerEvent::Completed, this, &ABoxelPlayerCharacter::TeamTalkInput_Released);
		
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ABoxelPlayerCharacter::PauseInput);
	}
}

void ABoxelPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, HeldItem);
}

