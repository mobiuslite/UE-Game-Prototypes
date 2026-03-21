#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EngineUtils.h"

#include "MobiusUtils.generated.h"

struct FFloatSpringState;
/**
 * 
 */
UCLASS()
class BOXEL_API UMobiusUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	//Creates start and end location for a trace directly out of the camera looking straight ahead
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool GetCameraTraceLocation(AController* Controller, float Distance, FVector& StartLocation, FVector& EndLocation);
	
	//Creates start and end location for a trace directly out of the camera looking straight using the control rotation
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool GetCameraControlTraceLocation(AController* Controller, float Distance, FVector& StartLocation, FVector& EndLocation);
	
	//Creates start and end location for a trace directly out of the camera looking straight ahead
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool GetCameraLocation(AController* Controller, FVector& Location);
	
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

	UFUNCTION(BlueprintCallable, meta=( WorldContext="WorldContextObject"))
	static void SetInputModeGameEnabled(const UObject* WorldContextObject, const bool bGameOnlyEnabled, const bool bFlushInput);
	
	UFUNCTION(BlueprintCallable)
	static void TickDownFloat(UPARAM(ref) float& Timer, const float DeltaTime, bool& bDone);
	
	template<class T>
	static T GetRandomItem(const TArray<T>& Array)
	{
		return Array[FMath::RandRange(0, Array.Num() - 1)];
	}
	
	template<class T>
	static TArray<T*> GetAllActorsOfClassEX(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass)
	{
		TArray<T*> Result;
		if (ActorClass)
		{
			if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
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
};
