#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EngineUtils.h"
#include "GenericTeamAgentInterface.h"

#include "MobiusUtils.generated.h"

class UInventoryComponent;
struct FFloatSpringState;

UCLASS()
class BOXEL_API UMobiusUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	//Creates start and end location for a trace directly out of the camera looking straight ahead
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool GetAimTraceLocation(AController* Controller, const float Distance, FVector& OutStartLocation, FVector& OutEndLocation);
	
	//Creates start and end location for a trace directly out of the camera looking straight using the control rotation
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool GetCameraControlTraceLocation(AController* Controller, const float Distance, FVector& OutStartLocation, FVector& OutEndLocation);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool GetCameraLocation(AController* Controller, FVector& OutLocation);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FVector AddVectorDirection(const FVector& Origin, const FVector& Direction, const float Distance);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FVector StableLerpVector(const FVector& Current, const FVector& Target, const float Exponent, const float DeltaSeconds)
	{
		return FMath::Lerp(Current, Target, 1.0f - FMath::Exp(-Exponent * DeltaSeconds));
	}
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FVector2D StableLerpVector2D(const FVector2D& Current, const FVector2D& Target, const float Exponent, const float DeltaSeconds)
	{
		return FMath::Lerp(Current, Target, 1.0f - FMath::Exp(-Exponent * DeltaSeconds));
	}
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float StableLerpFloat(const float Current, const float Target, const float Exponent, const float DeltaSeconds)
	{
		return FMath::Lerp(Current, Target, 1.0f - FMath::Exp(-Exponent * DeltaSeconds));
	}
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static double StableLerpDouble(const double Current, const double Target, const float Exponent, const float DeltaSeconds)
	{
		return FMath::Lerp(Current, Target, 1.0f - FMath::Exp(-Exponent * DeltaSeconds));
	}
	
	static float StableEaseOutFloat(const float Current, const float Target, const float Exponent, const float DeltaSeconds)
	{
		return FMath::InterpEaseOut(Current, Target, 1.0f - FMath::Exp(-Exponent * DeltaSeconds), 2.0f);
	}
	static float StableEaseInFloat(const float Current, const float Target, const float Exponent, const float DeltaSeconds)
	{
		return FMath::InterpEaseIn(Current, Target, 1.0f - FMath::Exp(-Exponent * DeltaSeconds), 2.0f);
	}
	
	UFUNCTION(BlueprintCallable)
	static void TickDownFloat(UPARAM(ref) float& Timer, const float DeltaTime, bool& bDone);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(Keywords="seconds, time, float"))
	static FString FloatToMinutesSeconds(float Seconds);
	
	template<class T>
	static T GetRandomItem(const TArray<T>& Array)
	{
		if (Array.Num() == 0) return T();
		
		return Array[FMath::RandRange(0, Array.Num() - 1)];
	}
	
	template<class T>
	static TArray<T> GetRandomItems(const TArray<T>& Array, const int RequestedAmount, const bool bCanHaveDuplicateEntries = false)
	{
		TArray<T> Results;
		if (Array.Num() == 0) return Results;	
		
		Results.SetNum(RequestedAmount, EAllowShrinking::No);
		
		TArray<T> PossibleItems;
		PossibleItems.Append(Array);

		for (int i = 0; i < RequestedAmount; ++i)
		{
			const int RandomIndex = FMath::RandRange(0, PossibleItems.Num() - 1);
			
			Results[i] = PossibleItems[RandomIndex];
			PossibleItems.RemoveAt(RandomIndex);
			
			if (PossibleItems.Num() == 0 && Results.Num() != RequestedAmount)
			{
				if (bCanHaveDuplicateEntries)
				{
					PossibleItems.Append(Array);
				}
				else
				{
					break;
				}
			}
		}
		
		return Results;
	}
	
	template<class T>
	static T* GetActorOfClassEX(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass)
	{
		if (ActorClass)
		{
			if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
			{
				if (const TActorIterator<AActor> It(World, ActorClass); It)
				{
					AActor* Actor = *It;
					return Cast<T>(Actor);
				}
			}
		}
		return nullptr;
	}
	
	template<class T>
	static TArray<T*> GetAllActorsOfClassEX(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass)
	{
		TArray<T*> Result;
		if (ActorClass)
		{
			if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
			{
				for (TActorIterator It(World, ActorClass); It; ++It)
				{
					AActor* Actor = *It;
					if (T* CastedActor = Cast<T>(Actor))
					{
						Result.Add(CastedActor);	
					}
					
				}
			}
		}
		return Result;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool GetInventory(const AActor* Actor, UInventoryComponent*& OutInventory);
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(DefaultToSelf = "Actor"))
	static FGenericTeamId GetTeamId(const AActor* Actor);
	
	UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs="OutAttitude"))
	static void GetTeamAttitudeExec(const FGenericTeamId& ThisTeamId, const FGenericTeamId& OtherTeamId, TEnumAsByte<ETeamAttitude::Type>& OutAttitude);
	UFUNCTION(BlueprintCallable)
	static ETeamAttitude::Type GetTeamAttitude(const FGenericTeamId& ThisTeamId, const FGenericTeamId& OtherTeamId);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool IsDead(const AActor* Actor);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool IsAlive(const AActor* Actor);
	
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void ServerTravel(const UObject* WorldContextObject, const FString& InURL, bool bAbsolute = false, bool bShouldSkipGameNotify = false);
};
