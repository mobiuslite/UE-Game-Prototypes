#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpleefGameMode.generated.h"

class USpleefModifier;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDiedSignature, AActor*, DeadPlayer);
UCLASS()
class BOXEL_API ASpleefGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION(BlueprintCallable)
	void StartSpleefGame();
	
	UFUNCTION(BlueprintCallable)
	void KillPlayer(AActor* Player);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int NumBots = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<APawn> BotClass; 
	
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
	
	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> CurrentAlivePlayers;
	
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite)
	FOnPlayerDiedSignature OnPlayerDiedDelegate;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpawnRadius = 1000.0f;
	
};
