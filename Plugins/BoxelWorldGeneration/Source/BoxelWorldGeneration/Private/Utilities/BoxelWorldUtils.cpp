// 2026 Mobius Lite Games (C) All Rights Reserved


#include "Utilities/BoxelWorldUtils.h"

#include "WorldGeneration/BoxelChunk.h"
#include "WorldGeneration/BoxelWorld.h"

FIntVector2 UBoxelWorldUtils::IndexToCoord(const int32 Index, const int32 ArrayLength)
{
	FIntVector2 Result = FIntVector2(-1);
	if (ArrayLength <= 0) return Result;
	
	Result.Y = Index / ArrayLength;
	Result.X = Index - Result.Y * ArrayLength;
	
	return Result;
}

int UBoxelWorldUtils::CoordToIndex(const FIntVector2& Coord, const int32 ArrayLength)
{
	int Result = -1;
	if (ArrayLength <= 0) return Result;
	if (Coord.X < 0 || Coord.Y < 0 || Coord.X >= ArrayLength || Coord.Y >= ArrayLength) return Result;

	Result = Coord.Y * ArrayLength + Coord.X;
	return Result;
}

FIntVector2 UBoxelWorldUtils::IndicesToGlobalCoord(const int32 ChunkIndex, const int32 BlockIndex, const int32 ChunkSize, const int32 WorldSize)
{
	const FIntVector2 BlockCoord = IndexToCoord(BlockIndex, ChunkSize);
	const FIntVector2 ChunkCoord = IndexToCoord(ChunkIndex, WorldSize);

	const FIntVector2 ChunkGlobalCoord = ChunkCoord * ChunkSize;
	
	return ChunkGlobalCoord + BlockCoord;
}

void UBoxelWorldUtils::GlobalCoordToIndices(const FIntVector2& GlobalCoord, const int32 ChunkSize,
	const int32 WorldSize, int32& OutChunkIndex, int32& OutBlockIndex)
{
	FIntVector2 ChunkCoord = GlobalCoord / ChunkSize;
	if (WorldSize == 1)
	{
		ChunkCoord = FIntVector2(0);
	}
	
	OutChunkIndex = CoordToIndex(ChunkCoord, WorldSize);
	OutBlockIndex = CoordToIndex(GlobalCoord - (ChunkCoord * ChunkSize), ChunkSize);
}

FVector UBoxelWorldUtils::BlockWorldLocation(const ABoxelWorld* WorldGen, const int32 ChunkIndex, const int32 BlockIndex)
{
	const FIntVector2 GlobalCoord = IndicesToGlobalCoord(ChunkIndex, BlockIndex, WorldGen->GetChunkSize(), WorldGen->GetWorldSize());
	
	const FVector2D BlockLocalLocation = (FVector2D)GlobalCoord * WorldGen->GetBlockScale();
	return WorldGen->GetActorLocation() + FVector(BlockLocalLocation.X, BlockLocalLocation.Y, 0.0f);
}

void UBoxelWorldUtils::WorldLocationToIndices(const ABoxelWorld* WorldGen, const FVector& WorldLocation,
	const ABoxelChunk* Chunk, int32& OutChunkIndex, int32& OutBlockIndex)
{
	const FVector LocalLocation = WorldLocation - Chunk->GetActorLocation();
	const FIntVector2 TruncatedLocation = FIntVector2(FMath::TruncToInt32(LocalLocation.X / WorldGen->GetBlockScale()), FMath::TruncToInt32(LocalLocation.Y / WorldGen->GetBlockScale()));
	
	GlobalCoordToIndices(TruncatedLocation, WorldGen->GetChunkSize(), WorldGen->GetWorldSize(), OutChunkIndex, OutBlockIndex);
	
	OutChunkIndex = Chunk->GetChunkIndex();
}
