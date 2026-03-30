// 


#include "Gameplay/AbilitySystem/HitscanGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayCueFunctionLibrary.h"
#include "Gameplay/Weapons/GunBase.h"
#include "MobiusAbilitySystem/Utils/MAUtils.h"
#include "Utility/MobiusCvars.h"
#include "Utility/MobiusGameplayTags.h"
#include "Utility/MobiusUtils.h"

void UHitscanGameplayAbility::FireGun(AGunBase* Gun)
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);
	
	OnTargetDataReadyCallbackDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);
	
	StartRangedWeaponTargeting();
}

void UHitscanGameplayAbility::OnFinishFire()
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	// When ability ends, consume target data and remove delegate
	MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
	MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

void UHitscanGameplayAbility::StartRangedWeaponTargeting()
{
	check(CurrentActorInfo);

	const AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());

	TArray<FHitResult> FoundHits = PerformLocalTargeting();


	FGameplayAbilityTargetDataHandle TargetData;
	if (FoundHits.Num() > 0)
	{
		for (const FHitResult& FoundHit : FoundHits)
		{
			FGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
			NewTargetData->HitResult = FoundHit;

			TargetData.Add(NewTargetData);
		}
	}

	// Process the target data immediately
	OnTargetDataReadyCallback(TargetData, FGameplayTag());
}

void UHitscanGameplayAbility::OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData)
{
	//Muzzle Cue
	if (const AGunBase* Gun = GetGunActor())
	{
		FGameplayCueParameters Params;
		Params.TargetAttachComponent = Gun->GetMuzzleComponent();
		Params.Location = Gun->GetMuzzleComponent()->GetComponentLocation();
		Params.Instigator = GetAvatarActorFromActorInfo();
		
		K2_ExecuteGameplayCueWithParams(Gun->GetFireCueTag(), Params);
	}
	
	//Impact cue
	const int32 DataCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(TargetData);
	for (int i = 0; i < DataCount; ++i)
	{
		const FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetData, i);
		if (HitResult.bBlockingHit)
		{
			const FGameplayCueParameters Params = UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(HitResult);
			K2_ExecuteGameplayCueWithParams(TAG_Gun_Bullet_Impact, Params);
		}
	}
	
	if (IsValid(DamageClass) && HasAuthority(&CurrentActivationInfo))
	{
		const FGameplayEffectSpecHandle Handle = UMAUtils::MakeHitDamageSpec(GetAbilitySystemComponentFromActorInfo(), this, DamageClass);
		K2_ApplyGameplayEffectSpecToTarget(Handle, TargetData);
	}
}

void UHitscanGameplayAbility::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData,
													FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	if (const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow	ScopedPrediction(MyAbilityComponent);

		// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
		if (bShouldNotifyServer)
		{
			MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, MyAbilityComponent->ScopedPredictionKey);
		}
		
		// See if we still have ammo
		if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			// Let the blueprint do stuff like apply effects to the targets
			OnRangedWeaponTargetDataReady(LocalTargetDataHandle);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Weapon ability %s failed to commit"), *GetPathName());
			K2_EndAbility();
		}
	}

	// We've processed the data
	MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

TArray<FHitResult> UHitscanGameplayAbility::PerformLocalTargeting()
{
	TArray<FHitResult> Results;
	
	const APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (AvatarPawn && AvatarPawn->IsLocallyControlled())
	{
		FRangedWeaponFiringInput InputData;
		
		const float Range = 99999.0f;
		UMobiusUtils::GetCameraControlTraceLocation(AvatarPawn->GetController(), Range, InputData.StartTrace, InputData.EndTrace);
		
		InputData.AimDir = (InputData.EndTrace - InputData.StartTrace).GetSafeNormal();

		TraceBulletsInCartridge(InputData,Results);
	}
	
	return Results;
}

FHitResult UHitscanGameplayAbility::DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const
{
	if (CVDebugDrawGunfire->GetBool())
	{
		static float DebugThickness = 1.0f;
		DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, 1.0f, 0, DebugThickness);
	}
	FHitResult Impact;

	// Trace and process instant hit if something was hit
	// First trace without using sweep radius
	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		Impact = WeaponTrace(StartTrace, EndTrace, 0.0f, bIsSimulated, OutHits);
	}

	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		// If this weapon didn't hit anything with a line trace and supports a sweep radius, try that
		if (SweepRadius > 0.0f)
		{
			TArray<FHitResult> SweepHits;
			Impact = WeaponTrace(StartTrace, EndTrace, SweepRadius, bIsSimulated, /*out*/ SweepHits);

			// If the trace with sweep radius enabled hit a pawn, check if we should use its hit results
			const int32 FirstPawnIdx = FindFirstPawnHitResult(SweepHits);
			if (SweepHits.IsValidIndex(FirstPawnIdx))
			{
				// If we had a blocking hit in our line trace that occurs in SweepHits before our
				// hit pawn, we should just use our initial hit results since the Pawn hit should be blocked
				bool bUseSweepHits = true;
				for (int32 Idx = 0; Idx < FirstPawnIdx; ++Idx)
				{
					const FHitResult& CurHitResult = SweepHits[Idx];

					auto Pred = [&CurHitResult](const FHitResult& Other)
					{
						return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
					};
					if (CurHitResult.bBlockingHit && OutHits.ContainsByPredicate(Pred))
					{
						bUseSweepHits = false;
						break;
					}
				}

				if (bUseSweepHits)
				{
					OutHits = SweepHits;
				}
			}
		}
	}

	return Impact;
}

FHitResult UHitscanGameplayAbility::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const
{
	TArray<FHitResult> HitResults;
	
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), /*bTraceComplex=*/ true, /*IgnoreActor=*/ GetAvatarActorFromActorInfo());
	TraceParams.bReturnPhysicalMaterial = true;
	AddAdditionalTraceIgnoreActors(TraceParams);
	//TraceParams.bDebugQuery = true;

	const ECollisionChannel TraceChannel = ECC_Pawn;

	if (SweepRadius > 0.0f)
	{
		GetWorld()->SweepMultiByChannel(HitResults, StartTrace, EndTrace, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(SweepRadius), TraceParams);
	}
	else
	{
		GetWorld()->LineTraceMultiByChannel(HitResults, StartTrace, EndTrace, TraceChannel, TraceParams);
	}

	FHitResult Hit(ForceInit);
	if (HitResults.Num() > 0)
	{
		// Filter the output list to prevent multiple hits on the same actor;
		// this is to prevent a single bullet dealing damage multiple times to
		// a single actor if using an overlap trace
		for (FHitResult& CurHitResult : HitResults)
		{
			auto Pred = [&CurHitResult](const FHitResult& Other)
			{
				return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
			};

			if (!OutHitResults.ContainsByPredicate(Pred))
			{
				OutHitResults.Add(CurHitResult);
			}
		}

		Hit = OutHitResults.Last();
	}
	else
	{
		Hit.TraceStart = StartTrace;
		Hit.TraceEnd = EndTrace;
	}

	return Hit;
}

void UHitscanGameplayAbility::AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		// Ignore any actors attached to the avatar doing the shooting
		TArray<AActor*> AttachedActors;
		Avatar->GetAttachedActors(/*out*/ AttachedActors);
		TraceParams.AddIgnoredActors(AttachedActors);
	}
}

int32 UHitscanGameplayAbility::FindFirstPawnHitResult(const TArray<FHitResult>& HitResults)
{
	for (int32 Idx = 0; Idx < HitResults.Num(); ++Idx)
	{
		const FHitResult& CurHitResult = HitResults[Idx];
		if (CurHitResult.HitObjectHandle.DoesRepresentClass(APawn::StaticClass()))
		{
			// If we hit a pawn, we're good
			return Idx;
		}
		else
		{
			AActor* HitActor = CurHitResult.HitObjectHandle.FetchActor();
			if ((HitActor != nullptr) && (HitActor->GetAttachParentActor() != nullptr) && (Cast<APawn>(HitActor->GetAttachParentActor()) != nullptr))
			{
				// If we hit something attached to a pawn, we're good
				return Idx;
			}
		}
	}

	return INDEX_NONE;
}

void UHitscanGameplayAbility::TraceBulletsInCartridge(const FRangedWeaponFiringInput& InputData,
	TArray<FHitResult>& OutHits)
{

	//TODO: Maybe add shotguns of some sort that allow multiple bullets in a cart
	//const int32 BulletsPerCartridge = WeaponData->GetBulletsPerCartridge();
	const int32 BulletsPerCartridge = 1;
	
	for (int32 BulletIndex = 0; BulletIndex < BulletsPerCartridge; ++BulletIndex)
	{
		//const float BaseSpreadAngle = WeaponData->GetCalculatedSpreadAngle();
		//const float SpreadAngleMultiplier = WeaponData->GetCalculatedSpreadAngleMultiplier();
		//const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;

		//const float HalfSpreadAngleInRadians = FMath::DegreesToRadians(ActualSpreadAngle * 0.5f);

		//const FVector BulletDir = VRandConeNormalDistribution(InputData.AimDir, HalfSpreadAngleInRadians, WeaponData->GetSpreadExponent());

		const FVector EndTrace = InputData.StartTrace + (InputData.AimDir * 999999.0f);
		FVector HitLocation = EndTrace;

		TArray<FHitResult> AllImpacts;

		FHitResult Impact = DoSingleBulletTrace(InputData.StartTrace, EndTrace, 0.0f, false,AllImpacts);

		const AActor* HitActor = Impact.GetActor();

		if (HitActor)
		{
			if (CVDebugDrawGunfire->GetBool())
			{
				DrawDebugPoint(GetWorld(), Impact.ImpactPoint, 25.0f, FColor::Red, false, 1.0f);
			}
			
			if (AllImpacts.Num() > 0)
			{
				OutHits.Append(AllImpacts);
			}

			HitLocation = Impact.ImpactPoint;
		}

		// Make sure there's always an entry in OutHits so the direction can be used for tracers, etc...
		if (OutHits.Num() == 0)
		{
			if (!Impact.bBlockingHit)
			{
				// Locate the fake 'impact' at the end of the trace
				Impact.Location = EndTrace;
				Impact.ImpactPoint = EndTrace;
			}

			OutHits.Add(Impact);
		}
	}
}