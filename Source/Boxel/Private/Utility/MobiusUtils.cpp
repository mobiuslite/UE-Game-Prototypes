// 2026 Mobius Lite Games (C) All Rights Reserved


#include "Utility/MobiusUtils.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

bool UMobiusUtils::GetCameraTraceLocation(AController* Controller, float const Distance, FVector& OutStartLocation, FVector& OutEndLocation)
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	
	if (!PlayerController) return false;
	if (!PlayerController->PlayerCameraManager) return false;
	if (!PlayerController->GetPawn()) return false;
	
	OutStartLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	const APawn* Pawn = PlayerController->GetPawn();
	const FRotator AimRotation = Pawn->GetBaseAimRotation();
	OutEndLocation = OutStartLocation + AimRotation.Vector() * Distance;
	
	return true;
}

bool UMobiusUtils::GetCameraControlTraceLocation(AController* Controller, const float Distance, FVector& OutStartLocation,
	FVector& OutEndLocation)
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	
	if (!PlayerController) return false;
	if (!PlayerController->PlayerCameraManager) return false;
	if (!PlayerController->GetPawn()) return false;
	
	OutStartLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	const APawn* Pawn = PlayerController->GetPawn();
	const FRotator AimRotation = Pawn->GetControlRotation();
	OutEndLocation = OutStartLocation + AimRotation.Vector() * Distance;
	
	return true;
}

bool UMobiusUtils::GetCameraLocation(AController* Controller, FVector& OutLocation)
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	
	if (!PlayerController) return false;
	if (!PlayerController->PlayerCameraManager) return false;
	OutLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	
	return true;
}

FVector UMobiusUtils::AddVectorDirection(const FVector& Origin, const FVector& Direction, const float Distance)
{
	return Origin + (Direction.GetSafeNormal() * Distance);
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

FString UMobiusUtils::FloatToMinutesSeconds(float Seconds)
{
	const TCHAR* NegativeModifier = Seconds < 0.f? TEXT("-") : TEXT("");
	Seconds = FMath::Abs(Seconds);
	
	const int32 NumMinutes = FMath::FloorToInt(Seconds/60.f);
	const int32 NumSeconds = FMath::FloorToInt(Seconds-(NumMinutes*60.f));
	
	return FString::Printf(TEXT("%s%02d:%02d"), NegativeModifier, NumMinutes, NumSeconds);
}
