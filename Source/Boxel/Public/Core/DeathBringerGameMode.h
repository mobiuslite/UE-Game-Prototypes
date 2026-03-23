// 

#pragma once

#include "CoreMinimal.h"
#include "MobiusGameMode.h"
#include "DeathBringerGameMode.generated.h"



UCLASS()
class BOXEL_API ADeathBringerGameMode : public AMobiusGameMode
{
	GENERATED_BODY()
	
public:
	
	virtual void BeginPlay() override;
	
	virtual void StartGame() override;
	
	static constexpr int32 DEATHBRINGER_TEAMID = 2;
	static constexpr int32 NORMALPLAYER_TEAMID = 1;
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DeathBringerPlayerRatio = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MinDeathBringers = 1;
	
	bool CanBeDeathBringer(const APlayerController* Controller) const;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<APlayerController*> DeathBringers;
};
