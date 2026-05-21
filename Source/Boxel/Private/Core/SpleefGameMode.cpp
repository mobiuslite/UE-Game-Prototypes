// 2026 Mobius Lite Games (C) All Rights Reserved


#include "Core/SpleefGameMode.h"

#include "Core/FrenzyModifiers/SpleefModifier.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "Gameplay/Player/BoxelPlayerController.h"
#include "Gameplay/WorldGen/WorldGenActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utility/MobiusUtils.h"

void ASpleefGameMode::BeginPlay()
{
	Super::BeginPlay();

	Modifiers.Empty();
	
	for (int i = 0; i < ModifierClasses.Num(); ++i)
	{
		Modifiers.Add(NewObject<USpleefModifier>(this, ModifierClasses[i]));
	}
}

void ASpleefGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	for (int i = 0; i < ActiveModifiers.Num(); ++i)
	{
		if (ActiveModifiers[i])
		{
			ActiveModifiers[i]->Tick(DeltaSeconds);	
		}
	}
}

void ASpleefGameMode::StartGame()
{
	Super::StartGame();
	
	
	TArray<AWorldGenActor*> WorldGenActors = UMobiusUtils::GetAllActorsOfClassEX<AWorldGenActor>(GetWorld(), AWorldGenActor::StaticClass());
	
	//Get the highest world gen, as that is the starting platform
	const AWorldGenActor* HighestWorldGen = nullptr;
	float HighestPosition = -999.0f;
	for (int i = 0; i < WorldGenActors.Num(); i++)
	{
		AWorldGenActor* WorldGen = WorldGenActors[i];
		WorldGen->MULTICAST_ResetWorld();
		
		if (!WorldGen) return;
		
		const FVector WorldLocation = WorldGen->GetActorLocation();
		if (WorldLocation.Z > HighestPosition)
		{
			HighestWorldGen = WorldGen;
			HighestPosition = WorldLocation.Z;
		}
	}
	
	if (!HighestWorldGen) return;
	
	if (AlivePlayers.Num() == 0) return;
	
	const FVector CenterLocation = HighestWorldGen->GetWorldCenter();
	const float AnglePerPlayer = 360.0f / AlivePlayers.Num();
	
	for (int i = 0; i < AlivePlayers.Num(); i++)
	{
		AActor* Player = AlivePlayers[i];
		if (!Player) return;
		
		float Angle = AnglePerPlayer * i;
		
		const float X = FMath::Cos(FMath::DegreesToRadians(Angle)) * SpawnRadius + CenterLocation.X;
		const float Y = FMath::Sin(FMath::DegreesToRadians(Angle)) * SpawnRadius + CenterLocation.X;
		const float Z = CenterLocation.Z + 300.0f;
		
		const FVector SpawnLocation(X, Y, Z);
		
		FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(Player->GetActorLocation(), CenterLocation);
		SpawnRotation.Pitch = 0.0f;
		SpawnRotation.Roll = 0.0f;
		
		Player->TeleportTo(SpawnLocation, SpawnRotation, false, true);
	}
	
	ApplyRandomModifiers(AlivePlayers);
}

void ASpleefGameMode::KillPlayer(APawn* Player, const AController* KilledBy)
{
	Super::KillPlayer(Player, KilledBy);
	
	if (AlivePlayers.Num() == 1)
	{
		OnPlayerWin(AlivePlayers[0]);
	}
	
	if (!Player->IsA(ABoxelPlayerCharacter::StaticClass()))
	{
		Player->Destroy();
	}
}

void ASpleefGameMode::ApplyRandomModifiers(const TArray<APawn*>& Players)
{
	DestroyCurrentModifiers();
	
	TArray<USpleefModifier*> PossibleModifiers;
	PossibleModifiers.Append(Modifiers);
	
	//TODO: Implement this
	const int NumRequestedModifiers = 1;
	const int NumModifiers = FMath::Clamp(NumRequestedModifiers, 1, PossibleModifiers.Num());
	
	FString ModifierString = "";
	for (int i = 0; i < NumModifiers; i++)
	{
		USpleefModifier* Modifier = UMobiusUtils::GetRandomItem(PossibleModifiers);
		
		ActiveModifiers.Add(Modifier);
		Modifier->ApplyModifier(Players);
		
		ModifierString.Append(Modifier->ModifierName);
		if (i != NumModifiers - 1)
		{
			ModifierString.Append("+");
		}
	}

	for (int i = 0; i < Players.Num(); ++i)
	{
		const APawn* Pawn = Cast<APawn>(Players[i]);
		if (!Pawn) continue;
		
		if (ABoxelPlayerController* Controller = Pawn->GetController<ABoxelPlayerController>())
		{
			Controller->ShowSpleefModifierName(ModifierString);
		}
	}
}

void ASpleefGameMode::DestroyCurrentModifiers()
{
	const TArray<APawn*> PlayerActors = UMobiusUtils::GetAllActorsOfClassEX<APawn>(GetWorld(), ABoxelPlayerCharacter::StaticClass());
	
	for (int i = 0; i < ActiveModifiers.Num(); ++i)
	{
		USpleefModifier* Modifier = ActiveModifiers[i];
		if (!Modifier) continue;
		
		Modifier->DestroyModifier(PlayerActors);
	}
	
	ActiveModifiers.Empty();
}

void ASpleefGameMode::OnPlayerWin(AActor* Winner)
{
	DestroyCurrentModifiers();
}
