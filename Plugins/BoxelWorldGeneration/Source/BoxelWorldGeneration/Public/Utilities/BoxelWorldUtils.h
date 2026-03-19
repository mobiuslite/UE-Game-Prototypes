// 2026 Mobius Lite Games (C) All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BoxelWorldUtils.generated.h"

class ABoxelChunk;
class ABoxelWorld;
/**
 * 
 */
UCLASS()
class BOXELWORLDGENERATION_API UBoxelWorldUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	static FIntVector2 IndexToCoord(const int32 Index, const int32 ArrayLength);
	static int CoordToIndex(const FIntVector2& Coord, const int32 ArrayLength);
	
	//Global coords are as if chunking didn't exist. E.g if chunk size is 10, a block coord of x = 5 in a chunk coord of x = 1 would be a global coord of x = 15
	static FIntVector2 IndicesToGlobalCoord(const int32 ChunkIndex, const int32 BlockIndex, const int32 ChunkLength, const int32 WorldLength);
	static void GlobalCoordToIndices(const FIntVector2& GlobalCoord, const int32 ChunkLength, const int32 WorldLength, int32& OutChunkIndex, int32& OutBlockIndex);
	
	static FVector BlockWorldLocation(const ABoxelWorld* WorldGen, const int32 ChunkIndex, const int32 BlockIndex);
	static void WorldLocationToIndices(const ABoxelWorld* WorldGen, const FVector& WorldLocation, const ABoxelChunk* Chunk, int32& OutChunkIndex, int32& OutBlockIndex);
};
