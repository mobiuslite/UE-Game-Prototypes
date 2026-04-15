// 


#include "Gameplay/DeathBringer/WeaponSpawner.h"

#include "Core/MobiusGameMode.h"
#include "Gameplay/Weapons/GunBase.h"
#include "Kismet/KismetMathLibrary.h"

AWeaponSpawner::AWeaponSpawner()
{

}

void AWeaponSpawner::ResetSpawner()
{
	if (!HasAuthority()) return;
	
	for (int i = 0; i < SpawnedActors.Num(); ++i)
	{
		FWeaponSpawnData& Data = SpawnedActors[i];
		
		if (IsValid(Data.Actor))
		{
			if (AGunBase* GunBase = Cast<AGunBase>(Data.Actor))
			{
				GunBase->ResetGun();
			}
			
			Data.Actor->SetActorLocation(Data.SpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
		}
		else if (Data.bIsRespawnable && IsValid(Data.Class))
		{
			FActorSpawnParameters Params;
			Data.Actor = GetWorld()->SpawnActor<AActor>(Data.Class, Data.SpawnLocation, FRotator::ZeroRotator, Params);
		}
	}
}

void AWeaponSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		{
			FWeaponSpawnData NewSpawnData;
		
			NewSpawnData.SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, SpreadAmount * 1.25f);
		
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			NewSpawnData.Actor = GetWorld()->SpawnActor<AGunBase>(GunClass, NewSpawnData.SpawnLocation, FRotator::ZeroRotator, Params);
		
			SpawnedActors.Add(NewSpawnData);
		}
		
		for (int i = 0; i < NumResources; ++i)
		{
			FWeaponSpawnData NewResourceData;
			NewResourceData.SpawnLocation = GetActorLocation() + (FMath::VRand() * FMath::RandRange(0, 1) * SpreadAmount);
			NewResourceData.SpawnLocation += FVector(0.0f, 0.0f, SpreadAmount);
			
			NewResourceData.bIsRespawnable = true;
			NewResourceData.Class = ResourceClass;
			
			FActorSpawnParameters Params;
			NewResourceData.Actor = GetWorld()->SpawnActor<AActor>(ResourceClass, NewResourceData.SpawnLocation, FRotator::ZeroRotator, Params);
		
			SpawnedActors.Add(NewResourceData);
		}
		
		if (AMobiusGameMode* GameMode = GetWorld()->GetAuthGameMode<AMobiusGameMode>())
		{
			GameMode->OnRoundHardResetDelegate.AddDynamic(this, &ThisClass::AUTH_OnRoundReset);
		}
	}
}

void AWeaponSpawner::AUTH_OnRoundReset()
{
	ResetSpawner();
}

