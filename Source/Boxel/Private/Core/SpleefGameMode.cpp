// 2026 Mobius Lite Games (C) All Rights Reserved


#include "Core/SpleefGameMode.h"

#include "Core/FrenzyModifiers/SpleefModifier.h"
#include "Gameplay/Player/BoxelPlayerCharacter.h"
#include "Gameplay/Player/BoxelPlayerController.h"
#include "Gameplay/WorldGen/WorldGenActor.h"
#include "Kismet/GameplayStatics.h"
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
	
	TArray<AActor*> WorldGenActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldGenActor::StaticClass(), WorldGenActors);
	
	//Get the highest world gen, as that is the starting platform
	const AWorldGenActor* HighestWorldGen = nullptr;
	float HighestPosition = -999.0f;
	for (int i = 0; i < WorldGenActors.Num(); i++)
	{
		AWorldGenActor* WorldGen = Cast<AWorldGenActor>(WorldGenActors[i]);
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
	
	if (CurrentAlivePlayers.Num() == 0) return;
	
	const FVector CenterLocation = HighestWorldGen->GetWorldCenter();
	const float AnglePerPlayer = 360.0f / CurrentAlivePlayers.Num();
	
	for (int i = 0; i < CurrentAlivePlayers.Num(); i++)
	{
		AActor* Player = CurrentAlivePlayers[i];
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
	
	ApplyRandomModifiers(CurrentAlivePlayers);
}

void ASpleefGameMode::KillPlayer(AActor* Player)
{
	Super::KillPlayer(Player);
	
	if (CurrentAlivePlayers.Num() == 1)
	{
		OnPlayerWin(CurrentAlivePlayers[0]);
	}
	
	if (!Player->IsA(ABoxelPlayerCharacter::StaticClass()))
	{
		Player->Destroy();
	}
}

void ASpleefGameMode::ApplyRandomModifiers(const TArray<AActor*>& Players)
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
	TArray<AActor*> PlayerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoxelPlayerCharacter::StaticClass(), PlayerActors);
	
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
