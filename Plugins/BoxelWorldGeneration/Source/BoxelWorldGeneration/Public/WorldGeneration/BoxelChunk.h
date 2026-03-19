// 2026 Mobius Lite Games (C) All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxelChunk.generated.h"

class UProceduralMeshComponent;
struct FBoxelBlock;
class ABoxelWorld;

UCLASS()
class BOXELWORLDGENERATION_API ABoxelChunk : public AActor
{
	GENERATED_BODY()
public:
	
	ABoxelChunk();
	void InitializeChunk(const int32 Index);
	void GenerateMesh();
	
	const TArray<FBoxelBlock>& GetBlocks() const;
	UProceduralMeshComponent* GetMesh() const;
	int32 GetChunkIndex() const { return ChunkIndex; }
	
	void AddBlock(FBoxelBlock Block);
	void SetBlockType(const int32 BlockIndex, const int32 BlockType);
	
	UFUNCTION(BlueprintCallable)
	void HitBlock(const FVector& WorldLocation);
	
protected:
	
	UPROPERTY()
	UProceduralMeshComponent* ChunkMesh;
	
	UPROPERTY()
	TArray<FBoxelBlock> Blocks;
	
	UPROPERTY()
	int32 ChunkIndex;
	
	UPROPERTY()
	ABoxelWorld* WorldGen;
};
