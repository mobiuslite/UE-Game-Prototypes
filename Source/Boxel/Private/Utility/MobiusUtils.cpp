// 2026 Mobius Lite Games (C) All Rights Reserved


#include "Utility/MobiusUtils.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

bool UMobiusUtils::GetCameraTraceLocation(AController* Controller, float Distance, FVector& StartLocation, FVector& EndLocation)
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	
	if (!PlayerController) return false;
	if (!PlayerController->PlayerCameraManager) return false;
	if (!PlayerController->GetPawn()) return false;
	
	StartLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	const APawn* Pawn = PlayerController->GetPawn();
	const FRotator AimRotation = Pawn->GetBaseAimRotation();
	EndLocation = StartLocation + AimRotation.Vector() * Distance;
	
	return true;
}

bool UMobiusUtils::GetCameraControlTraceLocation(AController* Controller, float Distance, FVector& StartLocation,
	FVector& EndLocation)
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	
	if (!PlayerController) return false;
	if (!PlayerController->PlayerCameraManager) return false;
	if (!PlayerController->GetPawn()) return false;
	
	StartLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	const APawn* Pawn = PlayerController->GetPawn();
	const FRotator AimRotation = Pawn->GetControlRotation();
	EndLocation = StartLocation + AimRotation.Vector() * Distance;
	
	return true;
}

bool UMobiusUtils::GetCameraLocation(AController* Controller, FVector& Location)
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	
	if (!PlayerController) return false;
	if (!PlayerController->PlayerCameraManager) return false;
	Location = PlayerController->PlayerCameraManager->GetCameraLocation();
	
	return true;
}

void UMobiusUtils::SetInputModeGameEnabled(const UObject* WorldContextObject, const bool bGameOnlyEnabled,
	const bool bFlushInput)
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (bGameOnlyEnabled)
		{
			UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController, bFlushInput);
		}
		else
		{
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PlayerController, nullptr, EMouseLockMode::DoNotLock, bFlushInput);
			
			FVector2D ScreenSize;
			GEngine->GameViewport->GetViewportSize(ScreenSize);
			
			PlayerController->SetMouseLocation(ScreenSize.X * 0.5f, ScreenSize.Y * 0.5f);
		}
		
		PlayerController->bShowMouseCursor = !bGameOnlyEnabled;
	}
}

void UMobiusUtils::TickDownFloat(float& Timer, const float DeltaTime, bool& bDone)
{
	bDone = false;
	Timer -= DeltaTime;
	if (Timer <= 0.0f)
	{
		bDone = true;
	}
}
