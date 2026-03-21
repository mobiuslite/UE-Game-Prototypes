// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

UCLASS()
class BOXEL_API AGunBase : public AActor
{
	GENERATED_BODY()

public:
	AGunBase();
	virtual void Tick(float DeltaTime) override;
	
	void SetHolder(APawn* HolderPawn);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APawn* GetHolder () const { return Holder; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsDroppable() const { return bDroppable; }
	
	void PullTrigger();
	void ReleaseTrigger();

protected:
	//Override to add functionality
	virtual void FireProjectile(const FVector& Location, const FRotator& Rotation) {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* GunMesh;
	
	UFUNCTION(Server, Reliable)
	void Server_FireProjectile(const FVector& Location, const FRotator& Rotation);
	
	UPROPERTY(EditDefaultsOnly, Category="Gun|Stats")
	float RPM = 400.0f;
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	bool bDroppable = true;
	
	UPROPERTY(BlueprintReadOnly)
	bool bTriggerDown;
	
	float FireTimer;
	
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	APawn* Holder;
};
