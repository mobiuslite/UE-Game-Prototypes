// 2026 Mobius Lite Games (C) All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BoxelMeshUtils.generated.h"

class ABoxelWorld;
class AWorldGenActor;
struct FProcMeshTangent;

USTRUCT()
struct FTriangleInfo
{
	GENERATED_BODY()
	
	FTriangleInfo() = default;
	
	//Requires points to be in counter-clock wise order
	FTriangleInfo(const FVector& Point0, const FVector& Point1, const FVector& Point2,
		const FVector2D& PointUV0, const FVector2D& PointUV1, const FVector2D& PointUV2,
		const FLinearColor& PointColor0 = FLinearColor(1.0f, 1.0f, 1.0f), 
		const FLinearColor& PointColor1 = FLinearColor(1.0f, 1.0f, 1.0f), 
		const FLinearColor& PointColor2 = FLinearColor(1.0f, 1.0f, 1.0f))
	{
		P0 = Point0;
		P1 = Point1;
		P2 = Point2;
		
		UV0 = PointUV0;
		UV1 = PointUV1;
		UV2 = PointUV2;
		
		Color0 = PointColor0;
		Color1 = PointColor1;
		Color2 = PointColor2;
	}

	UPROPERTY()
	FVector P0;
	UPROPERTY()
	FVector P1;
	UPROPERTY()
	FVector P2;
	
	UPROPERTY()
	FVector2D UV0;
	UPROPERTY()
	FVector2D UV1;
	UPROPERTY()
	FVector2D UV2;
	
	UPROPERTY()
	FLinearColor Color0;
	UPROPERTY()
	FLinearColor Color1;
	UPROPERTY()
	FLinearColor Color2;
};

USTRUCT()
struct FNeighbourInfo
{
	GENERATED_BODY()
	
	//-1 means the front block has the same chunk index as the middle block 
	static const int32 NO_CHUNK_NEIGHBOUR;
	
	UPROPERTY()
	bool bHasFrontBlockNeighbour = false;
	UPROPERTY()
	int32 FrontChunkIndex = NO_CHUNK_NEIGHBOUR;
	
	UPROPERTY()
	bool bHasBackBlockNeighbour = false;
	UPROPERTY()
	int32 BackChunkIndex = NO_CHUNK_NEIGHBOUR;
	
	UPROPERTY()
	bool bHasLeftBlockNeighbour = false;
	UPROPERTY()
	int32 LeftChunkIndex = NO_CHUNK_NEIGHBOUR;
	
	UPROPERTY()
	bool bHasRightBlockNeighbour = false;
	UPROPERTY()
	int32 RightChunkIndex = NO_CHUNK_NEIGHBOUR;
	
	UPROPERTY()
	bool bHasUpBlockNeighbour = false;
	UPROPERTY()
	int32 UpChunkIndex = NO_CHUNK_NEIGHBOUR;
	
	UPROPERTY()
	bool bHasDownBlockNeighbour = false;
	UPROPERTY()
	int32 DownChunkIndex = NO_CHUNK_NEIGHBOUR;
	
	TArray<int32> GetUniqueChunkNeighbourList() const;
};

UCLASS()
class BOXELWORLDGENERATION_API UBoxelMeshUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	static void CreateTriangle(const FTriangleInfo& TriangleInfo, 
		TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals,
		TArray<FProcMeshTangent>& Tangents, TArray<FVector2D>& UV0, TArray<FLinearColor>& VertexColors);
	
	static void CreateBox(ABoxelWorld* WorldGen, const int32 ChunkIndex, const int32 BlockIndex, const float BoxScale, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals,
		TArray<FProcMeshTangent>& Tangents, TArray<FVector2D>& UV0, TArray<FLinearColor>& VertexColors);
	
	static FNeighbourInfo HasNeighbour(const ABoxelWorld* WorldGen, const int32 ChunkIndex, const int32 BlockIndex);
	
};
