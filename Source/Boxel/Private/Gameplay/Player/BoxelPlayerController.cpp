// 


#include "Boxel/Public/Gameplay/Player/BoxelPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void ABoxelPlayerController::ShowSpleefModifierName_Implementation(const FString& Name)
{
	BP_ShowSpleefModifierName(Name);
}

void ABoxelPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!InputMapping.IsNull())
	{
		if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), 0);
			}
		}
	}
}
