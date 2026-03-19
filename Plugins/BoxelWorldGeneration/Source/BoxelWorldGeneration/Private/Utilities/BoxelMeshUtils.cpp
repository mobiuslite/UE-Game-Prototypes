// 2026 Mobius Lite Games (C) All Rights Reserved


#include "Utilities/BoxelMeshUtils.h"

#include "ProceduralMeshComponent.h"
#include "Utilities/BoxelWorldUtils.h"
#include "WorldGeneration/BoxelBlock.h"
#include "WorldGeneration/BoxelWorld.h"

const int32 FNeighbourInfo::NO_CHUNK_NEIGHBOUR = -1;

TArray<int32> FNeighbourInfo::GetUniqueChunkNeighbourList() const
{
	TArray<int32> Result;
	
	if (FrontChunkIndex != NO_CHUNK_NEIGHBOUR)
	{
		Result.AddUnique(FrontChunkIndex);
	}
	if (BackChunkIndex != NO_CHUNK_NEIGHBOUR)
	{
		Result.AddUnique(BackChunkIndex);
	}
	if (LeftChunkIndex != NO_CHUNK_NEIGHBOUR)
	{
		Result.AddUnique(LeftChunkIndex);
	}
	if (RightChunkIndex != NO_CHUNK_NEIGHBOUR)
	{
		Result.AddUnique(RightChunkIndex);
	}
	if (UpChunkIndex != NO_CHUNK_NEIGHBOUR)
	{
		Result.AddUnique(UpChunkIndex);
	}
	if (DownChunkIndex != NO_CHUNK_NEIGHBOUR)
	{
		Result.AddUnique(DownChunkIndex);
	}

	return Result;
}

void UBoxelMeshUtils::CreateTriangle(const FTriangleInfo& TriangleInfo, TArray<FVector>& Vertices,
                                     TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FProcMeshTangent>& Tangents, TArray<FVector2D>& UV0,
                                     TArray<FLinearColor>& VertexColors)
{
	Vertices.Add(TriangleInfo.P0);
	Vertices.Add(TriangleInfo.P1);
	Vertices.Add(TriangleInfo.P2);
	
	int32 LastTriangleIndex = Triangles.Num() == 0 ? -1 : Triangles.Last();
	
	Triangles.Add(++LastTriangleIndex);
	Triangles.Add(++LastTriangleIndex);
	Triangles.Add(++LastTriangleIndex);
	
	const FVector Normal = FVector::CrossProduct(TriangleInfo.P2 - TriangleInfo.P0, TriangleInfo.P1 - TriangleInfo.P0).GetSafeNormal();
	
	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);
	
	FVector Tangent = (TriangleInfo.P1 - TriangleInfo.P0).GetSafeNormal();
	Tangents.Add(FProcMeshTangent(Tangent[0], Tangent[1], Tangent[2]));
	Tangent = (TriangleInfo.P2 - TriangleInfo.P1).GetSafeNormal();
	Tangents.Add(FProcMeshTangent(Tangent[0], Tangent[1], Tangent[2]));
	Tangent = (TriangleInfo.P0 - TriangleInfo.P2).GetSafeNormal();
	Tangents.Add(FProcMeshTangent(Tangent[0], Tangent[1], Tangent[2]));
	
	UV0.Add(TriangleInfo.UV0);
	UV0.Add(TriangleInfo.UV1);
	UV0.Add(TriangleInfo.UV2);
	
	VertexColors.Add(TriangleInfo.Color0);
	VertexColors.Add(TriangleInfo.Color1);
	VertexColors.Add(TriangleInfo.Color2);
}

void UBoxelMeshUtils::CreateBox(ABoxelWorld* WorldGen, const int32 ChunkIndex, const int32 BlockIndex, 
	const float BoxScale, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals,
	TArray<FProcMeshTangent>& Tangents, TArray<FVector2D>& UV0, TArray<FLinearColor>& VertexColors)
{
	FIntVector2 BlockCoord = UBoxelWorldUtils::IndexToCoord(BlockIndex, WorldGen->GetChunkSize());
	
	FVector WorldLocation;
	WorldLocation.X = BlockCoord.X * BoxScale;
	WorldLocation.Y = BlockCoord.Y * BoxScale;
	WorldLocation.Z = 0.0f;
	
	const FNeighbourInfo Neighbours = UBoxelMeshUtils::HasNeighbour(WorldGen, ChunkIndex, BlockIndex);
	
	if (!Neighbours.bHasFrontBlockNeighbour)
	{
		//Front
		FTriangleInfo TriangleInfo = FTriangleInfo(
			BoxScale * FVector(0.0f, 0.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(0.0f, 1.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(0.0f, 1.0f, 1.0f) + WorldLocation,
			FVector2D(0.0f, 1.0f),
			FVector2D(1.0f, 1.0f),
			FVector2D(1.0f, 0.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	
		TriangleInfo = FTriangleInfo(
			BoxScale * FVector(0.0f, 0.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(0.0f, 1.0f, 1.0f) + WorldLocation,
			BoxScale * FVector(0.0f, 0.0f, 1.0f) + WorldLocation,
			FVector2D(0.0f, 1.0f),
			FVector2D(1.0f, 0.0f),
			FVector2D(0.0f, 0.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	}
	
	if (!Neighbours.bHasBackBlockNeighbour)
	{
		//Back
		FTriangleInfo TriangleInfo = FTriangleInfo(
			BoxScale * FVector(1.0f, 0.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 1.0f, 1.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 1.0f, 0.0f) + WorldLocation,
			FVector2D(1.0f, 1.0f),
			FVector2D(0.0f, 0.0f),
			FVector2D(0.0f, 1.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	
		TriangleInfo = FTriangleInfo(
			BoxScale * FVector(1.0f, 0.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 0.0f, 1.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 1.0f, 1.0f) + WorldLocation,
			FVector2D(1.0f, 1.0f),
			FVector2D(1.0f, 0.0f),
			FVector2D(0.0f, 0.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	}
	
	if (!Neighbours.bHasLeftBlockNeighbour)
	{
		//Left
		FTriangleInfo TriangleInfo = FTriangleInfo(
			BoxScale * FVector(0.0f, 0.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 0.0f, 1.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 0.0f, 0.0f) + WorldLocation,
			FVector2D(1.0f, 1.0f),
			FVector2D(0.0f, 0.0f),
			FVector2D(0.0f, 1.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	
		TriangleInfo = FTriangleInfo(
			BoxScale * FVector(0.0f, 0.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(0.0f, 0.0f, 1.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 0.0f, 1.0f) + WorldLocation,
			FVector2D(1.0f, 1.0f),
			FVector2D(1.0f, 0.0f),
			FVector2D(0.0f, 0.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	}
	
	if (!Neighbours.bHasRightBlockNeighbour)
	{
		//Right
		FTriangleInfo TriangleInfo = FTriangleInfo(
			BoxScale * FVector(0.0f, 1.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 1.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 1.0f, 1.0f) + WorldLocation,
			FVector2D(0.0f, 1.0f),
			FVector2D(1.0f, 1.0f),
			FVector2D(1.0f, 0.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	
		TriangleInfo = FTriangleInfo(
			BoxScale * FVector(0.0f, 1.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 1.0f, 1.0f) + WorldLocation,
			BoxScale * FVector(0.0f, 1.0f, 1.0f) + WorldLocation,
			FVector2D(0.0f, 1.0f),
			FVector2D(1.0f, 0.0f),
			FVector2D(0.0f, 0.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	}
	
	if (!Neighbours.bHasUpBlockNeighbour)
	{
		//Top
		FTriangleInfo TriangleInfo = FTriangleInfo(
			BoxScale * FVector(0.0f, 0.0f, 1.0f) + WorldLocation,
			BoxScale * FVector(0.0f, 1.0f, 1.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 0.0f, 1.0f) + WorldLocation,
			FVector2D(0.0f, 1.0f),
			FVector2D(1.0f, 1.0f),
			FVector2D(0.0f, 0.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	
		TriangleInfo = FTriangleInfo(
			BoxScale * FVector(1.0f, 1.0f, 1.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 0.0f, 1.0f) + WorldLocation,
			BoxScale * FVector(0.0f, 1.0f, 1.0f) + WorldLocation,
			FVector2D(1.0f, 0.0f),
			FVector2D(0.0f, 0.0f),
			FVector2D(1.0f, 1.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	}
	
	if (!Neighbours.bHasDownBlockNeighbour)
	{
		//Bottom
		FTriangleInfo TriangleInfo = FTriangleInfo(
			BoxScale * FVector(0.0f, 0.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 0.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(0.0f, 1.0f, 0.0f) + WorldLocation,
			FVector2D(0.0f, 0.0f),
			FVector2D(0.0f, 1.0f),
			FVector2D(1.0f, 0.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	
		TriangleInfo = FTriangleInfo(
			BoxScale * FVector(1.0f, 1.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(0.0f, 1.0f, 0.0f) + WorldLocation,
			BoxScale * FVector(1.0f, 0.0f, 0.0f) + WorldLocation,
			FVector2D(1.0f, 1.0f),
			FVector2D(1.0f, 0.0f),
			FVector2D(0.0f, 1.0f));
	
		UBoxelMeshUtils::CreateTriangle(TriangleInfo, Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
	}
}

FNeighbourInfo UBoxelMeshUtils::HasNeighbour(const ABoxelWorld* WorldGen, const int32 ChunkIndex, const int32 BlockIndex)
{
	FNeighbourInfo Result;
	
	FIntVector2 GlobalCoord = UBoxelWorldUtils::IndicesToGlobalCoord(ChunkIndex, BlockIndex, WorldGen->GetChunkSize(), WorldGen->GetWorldSize());
	
	if (const FBoxelBlock* FrontBlock = WorldGen->GetBlockFromGlobalCoord(GlobalCoord + FIntVector2(-1, 0)))
	{
		Result.bHasFrontBlockNeighbour = FrontBlock->BlockType != 0;
		
		if (ChunkIndex != FrontBlock->ChunkIndex)
		{
			Result.FrontChunkIndex = FrontBlock->ChunkIndex;
		}
	}
	if (const FBoxelBlock* BackBlock = WorldGen->GetBlockFromGlobalCoord(GlobalCoord + FIntVector2(1, 0)))
	{
		Result.bHasBackBlockNeighbour = BackBlock->BlockType != 0;
		if (ChunkIndex != BackBlock->ChunkIndex)
		{
			Result.BackChunkIndex = BackBlock->ChunkIndex;
		}
	}
	
	if (const FBoxelBlock* RightBlock = WorldGen->GetBlockFromGlobalCoord(GlobalCoord + FIntVector2(0, 1)))
	{
		Result.bHasRightBlockNeighbour = RightBlock->BlockType != 0;
		if (ChunkIndex != RightBlock->ChunkIndex)
		{
			Result.RightChunkIndex = RightBlock->ChunkIndex;
		}
	}
	if (const FBoxelBlock* LeftBlock = WorldGen->GetBlockFromGlobalCoord(GlobalCoord + FIntVector2(0, -1)))
	{
		Result.bHasLeftBlockNeighbour = LeftBlock->BlockType != 0;
		if (ChunkIndex != LeftBlock->ChunkIndex)
		{
			Result.LeftChunkIndex = LeftBlock->ChunkIndex;
		}
	}
	
	return Result;
}
