
#include "WorldGeneration/BoxelChunk.h"


#include "ProceduralMeshComponent.h"
#include "AI/NavigationSystemBase.h"
#include "Utilities/BoxelMeshUtils.h"
#include "Utilities/BoxelWorldUtils.h"
#include "WorldGeneration/BoxelBlock.h"
#include "WorldGeneration/BoxelWorld.h"

ABoxelChunk::ABoxelChunk()
{
	PrimaryActorTick.bCanEverTick = false;
	
	ChunkMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(ChunkMesh);
	
	ChunkMesh->SetCanEverAffectNavigation(true);
}

void ABoxelChunk::InitializeChunk(const int32 Index)
{
	ChunkIndex = Index;
	ChunkMesh->ClearAllMeshSections();
	
	WorldGen = Cast<ABoxelWorld>(GetOwner());
}

void ABoxelChunk::GenerateMesh()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;
	
	TArray<FVector2D> UV0;
	TArray<FLinearColor> VertexColors;
		
	for (int32 BlockIndex = 0; BlockIndex < Blocks.Num(); BlockIndex++)
	{
		const FBoxelBlock& Block = Blocks[BlockIndex];
		if (Block.BlockType != 0)
		{
			UBoxelMeshUtils::CreateBox(WorldGen, ChunkIndex, BlockIndex, WorldGen->GetBlockScale(), Vertices, Triangles, Normals, Tangents, UV0, VertexColors);
		}
	}
	
	if (ChunkMesh)
	{
		ChunkMesh->ClearAllMeshSections();
		ChunkMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, true);
	}
	
	FNavigationSystem::UpdateComponentData(*ChunkMesh);
}

const TArray<FBoxelBlock>& ABoxelChunk::GetBlocks() const
{
	return Blocks;
}

UProceduralMeshComponent* ABoxelChunk::GetMesh() const
{
	return ChunkMesh;
}

void ABoxelChunk::AddBlock(FBoxelBlock Block)
{
	Blocks.Add(Block);
}

void ABoxelChunk::SetBlockType(const int32 BlockIndex, const int32 BlockType)
{
	Blocks[BlockIndex].BlockType = BlockType;
}

void ABoxelChunk::HitBlock(const FVector& WorldLocation)
{
	if (!WorldGen->HasAuthority()) return;
	
	int32 OutChunkIndex;
	int32 OutBlockIndex;
	UBoxelWorldUtils::WorldLocationToIndices(WorldGen, WorldLocation, this, OutChunkIndex, OutBlockIndex);
	
	WorldGen->MULTICAST_DestroyBlock(this->ChunkIndex, OutBlockIndex);
}