// 2026 Mobius Lite Games (C) All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BoxelChunk.h"
#include "GameFramework/Actor.h"
#include "BoxelWorld.generated.h"

UCLASS()
class BOXELWORLDGENERATION_API ABoxelWorld : public AActor
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	ABoxelWorld();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
public:
	
	UFUNCTION(NetMulticast, Reliable)
	void MULTICAST_DestroyBlock(const int32 ChunkIndex, const int32 BlockIndex);
	
	UFUNCTION(NetMulticast, Reliable)
	void MULTICAST_ResetWorld();
	
	FVector GetWorldCenter() const;
	
	const ABoxelChunk* GetChunk(const FIntVector2 ChunkCoord) const;
	const ABoxelChunk* GetChunk(const int32 ChunkIndex) const;

	void RegenerateChunk(const int32 ChunkIndex) const;
	
	const FBoxelBlock* GetBlock(const int32 ChunkIndex, const int32 BlockIndex) const;
	const FBoxelBlock* GetBlockFromGlobalCoord(const FIntVector2& GlobalCoord) const;
	
	int32 GetChunkSize() const {return ChunkSize;}
	int32 GetWorldSize() const {return WorldSize;}
	
	float GetBlockScale() const {return BlockScale;}
	
protected:
	ABoxelChunk* GetMutableChunk(const int32 ChunkIndex) const;
	
	//Override in functions to determine world generation
	UFUNCTION(BlueprintImplementableEvent)
	bool ShouldGenerateBlock(const FVector& BlockWorldLocation, UPARAM(ref) FRandomStream& RandomStream);
	
	void GenerateWorld(int32 Seed);
	void GenerateWorldMesh();
	
	UPROPERTY()
	TArray<ABoxelChunk*> Chunks;
	void ClearChunks();
	
	//Length/Width of world in chunks
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WorldGen|Params")
	int32 WorldSize = 10;
	
	//Length/Width of chunk in blocks
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WorldGen|Params")
	int32 ChunkSize = 16;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units="Centimeters"), Category="WorldGen|Params")
	float BlockScale = 100.0f;
};
