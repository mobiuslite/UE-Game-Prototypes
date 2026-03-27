// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

class UGameplayAbility;
class UGunGameplayAbility;

USTRUCT()
struct FHolderHistoryData
{
	GENERATED_BODY()
	
	UPROPERTY()
	const APawn* PreviousHolder;
	UPROPERTY()
	float HeldCooldownTimer;
};

UCLASS()
class BOXEL_API AGunBase : public AActor
{
	GENERATED_BODY()

public:
	AGunBase();
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetHolder(APawn* HolderPawn);
	virtual void RemoveHolder(const APawn* HolderPawn, const bool bThrow = true);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APawn* GetHolder () const { return Holder; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsDroppable() const { return bDroppable; }
	bool CanBePickedUp(const APawn* PawnHolder) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetFireRate() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float IsFullyAutomatic() const { return bFullyAutomatic; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FGameplayTag GetFireCueTag() const { return OnFireGameplayCueTag; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	USceneComponent* GetMuzzleComponent() const { return MuzzleLocation; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TSubclassOf<UAnimInstance> GetGunAnimInstanceClass() const { return GunABP; }
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* GunMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* MuzzleLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gun|Animation")
	TSubclassOf<UAnimInstance> GunABP;

	//TODO: Using RPM like this would allow clients to set their own RPM and make damage cheats through it
	UPROPERTY(EditDefaultsOnly, Category="Gun|Stats")
	float RPM = 400.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Stats")
	bool bFullyAutomatic;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Throwing")
	bool bDroppable = true;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Throwing")
	float DropImpulseStrength = 240.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun|Throwing")
	float DropThrowOffset = 50.0f;
	
	//TODO: Implement
	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere, Category = "Gun|Ammo")
	int CurrentAmmo = 0;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|Abilities")
	TSubclassOf<UGameplayAbility> GrantedAbilityClass;
	UPROPERTY()
	FGameplayAbilitySpecHandle AbilityHandle;
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|FXs")
	FGameplayTag OnFireGameplayCueTag;
	
	virtual void BeginPlay() override;
private:
	
	UPROPERTY()
	APawn* Holder;
	
	void SetPhysicsEnabled(const bool bEnabled);
	
	//Stop players who just threw the gun from picking it up immediately 
	UPROPERTY()
	TArray<FHolderHistoryData> HolderHistory;
};
