// 

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Core/MobiusGameMode.h"
#include "LobbyGameMode.generated.h"

class UObjectLibrary;
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Lobby_InArena);

UCLASS()
class BOXEL_API ALobbyGameMode : public AMobiusGameMode
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable)
	void AddPlayerToArena(APlayerController* Controller);
	UFUNCTION(BlueprintCallable)
	void RemovePlayerFromArena(APlayerController* Controller);
	
	virtual void KillPlayer(APawn* Player, const AController* KilledBy) override;
	
protected:
	virtual void OnPlayerLogin(AGameModeBase* GameMode, APlayerController* PC) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName = L"") override;
	
	void GivePlayerGun(APawn* Pawn);
	
	UPROPERTY(EditDefaultsOnly)
	UObjectLibrary* ArenaGuns;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ArenaRespawnTimer = 2.0f;
	
	UPROPERTY()
	TArray<APlayerController*> ArenaPlayers;
};
