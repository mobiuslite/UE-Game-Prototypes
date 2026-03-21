#pragma once

#include "CoreMinimal.h"
#include "MobiusGameMode.h"
#include "SpleefGameMode.generated.h"

class USpleefModifier;

UCLASS()
class BOXEL_API ASpleefGameMode : public AMobiusGameMode
{
	GENERATED_BODY()
	
public:
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void StartGame() override;
	virtual void KillPlayer(AActor* Player) override;
	
protected:
	
	void ApplyRandomModifiers(const TArray<AActor*>& Players);
	void DestroyCurrentModifiers();
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TArray<TSubclassOf<USpleefModifier>> ModifierClasses;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<USpleefModifier*> Modifiers;
	UPROPERTY(BlueprintReadOnly)
	TArray<USpleefModifier*> ActiveModifiers;
	
	void OnPlayerWin(AActor* Winner);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpawnRadius = 1000.0f;
	
};
