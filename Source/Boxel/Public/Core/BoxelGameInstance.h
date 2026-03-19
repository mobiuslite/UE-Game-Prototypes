#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BoxelGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BOXEL_API UBoxelGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	
protected:
	
	UFUNCTION()
	void OnJoinRequested(int64 SteamID, const FString& ConnectionString);
};
