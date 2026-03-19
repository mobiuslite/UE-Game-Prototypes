
#include "WorldGeneration/BoxelWorld.h"

#include "ProceduralMeshComponent.h"
#include "Utilities/BoxelMeshUtils.h"
#include "Utilities/BoxelWorldUtils.h"
#include "WorldGeneration/BoxelBlock.h"
#include "WorldGeneration/BoxelChunk.h"


ABoxelWorld::ABoxelWorld()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void ABoxelWorld::BeginPlay()
{
	Super::BeginPlay();
	
	GenerateWorld(0);
}

void ABoxelWorld::Destroyed()
{
	Super::Destroyed();
	ClearChunks();
}

void ABoxelWorld::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ABoxelWorld::MULTICAST_DestroyBlock_Implementation(const int32 ChunkIndex, const int32 BlockIndex)
{
	ABoxelChunk* Chunk = GetMutableChunk(ChunkIndex);
	if (!Chunk) return;
	
	Chunk->SetBlockType(BlockIndex, 0);
	Chunk->GenerateMesh();
	
	const FNeighbourInfo Neighbours = UBoxelMeshUtils::HasNeighbour(this, ChunkIndex, BlockIndex);
	TArray<int32> NeighbourChunks = Neighbours.GetUniqueChunkNeighbourList();
	for (int i = 0; i < NeighbourChunks.Num(); i++)
	{
		const int32 NeighbourChunkIndex = NeighbourChunks[i];
		if (NeighbourChunkIndex == ChunkIndex) continue;
		RegenerateChunk(NeighbourChunkIndex);
	}
}

void ABoxelWorld::MULTICAST_ResetWorld_Implementation()
{
	GenerateWorld(0);
}

FVector ABoxelWorld::GetWorldCenter() const
{
	const FVector WorldLocation = GetActorLocation();
	const float HalfWorldLength = GetBlockScale() * GetChunkSize() * GetWorldSize() * 0.5f;
	
	return WorldLocation + FVector(HalfWorldLength, HalfWorldLength, 0.0f);
}

const ABoxelChunk* ABoxelWorld::GetChunk(const FIntVector2 ChunkCoord) const
{
	return GetChunk(UBoxelWorldUtils::CoordToIndex(ChunkCoord, WorldSize));
}

const ABoxelChunk* ABoxelWorld::GetChunk(const int32 ChunkIndex) const
{
	if (ChunkIndex < 0 || ChunkIndex >= Chunks.Num()) return nullptr;
	
	return Chunks[ChunkIndex];
}

void ABoxelWorld::RegenerateChunk(const int32 ChunkIndex) const
{
	if (ABoxelChunk* Chunk = GetMutableChunk(ChunkIndex))
	{
		Chunk->GenerateMesh();
	}
}

const FBoxelBlock* ABoxelWorld::GetBlock(const int32 ChunkIndex, const int32 BlockIndex) const
{
	if (ChunkIndex < 0 || ChunkIndex >= Chunks.Num()) return nullptr;
	if (BlockIndex < 0 || BlockIndex >= Chunks[ChunkIndex]->GetBlocks().Num()) return nullptr;
	
	return &Chunks[ChunkIndex]->GetBlocks()[BlockIndex];
}

const FBoxelBlock* ABoxelWorld::GetBlockFromGlobalCoord(const FIntVector2& GlobalCoord) const
{
	int32 ChunkIndex;
	int32 BlockIndex;
	UBoxelWorldUtils::GlobalCoordToIndices(GlobalCoord, GetChunkSize(), GetWorldSize(), ChunkIndex, BlockIndex);
	
	return GetBlock(ChunkIndex, BlockIndex);
}

ABoxelChunk* ABoxelWorld::GetMutableChunk(const int32 ChunkIndex) const
{
	if (ChunkIndex < 0 || ChunkIndex >= Chunks.Num()) return nullptr;
	
	return Chunks[ChunkIndex];
}

void ABoxelWorld::GenerateWorld(int32 Seed)
{
	FRandomStream RandomStream(Seed);
	
	ClearChunks();
	
	//Generate World
	for (int32 ChunkIndex = 0; ChunkIndex < WorldSize * WorldSize; ChunkIndex++)
	{
		const FIntVector2 ChunkCoord = UBoxelWorldUtils::IndexToCoord(ChunkIndex, GetWorldSize());
		
		FVector ChunkWorldLocation;
		ChunkWorldLocation.X = ChunkCoord.X * GetBlockScale() * GetChunkSize() + GetActorLocation().X;
		ChunkWorldLocation.Y = ChunkCoord.Y * GetBlockScale() * GetChunkSize() + GetActorLocation().Y;
		ChunkWorldLocation.Z = GetActorLocation().Z;
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;	// We never want these to save into a map
		
		ABoxelChunk* NewChunk = GetWorld()->SpawnActor<ABoxelChunk>(ChunkWorldLocation, FRotator::ZeroRotator, SpawnParams);
		NewChunk->InitializeChunk(ChunkIndex);
		
		for (int32 BlockIndex = 0; BlockIndex < ChunkSize * ChunkSize; BlockIndex++)
		{
			FVector BlockWorldLocation = UBoxelWorldUtils::BlockWorldLocation(this, ChunkIndex, BlockIndex);
			
			FBoxelBlock NewBlock;
			NewBlock.BlockIndex = BlockIndex;
			NewBlock.ChunkIndex = ChunkIndex;
			NewBlock.BlockType = ShouldGenerateBlock(BlockWorldLocation, RandomStream) ? 1 : 0;
			
			NewChunk->AddBlock(NewBlock);
		}
		
		Chunks.Add(NewChunk);
	}
	
	GenerateWorldMesh();
}

void ABoxelWorld::GenerateWorldMesh()
{
	for (int32 ChunkIndex = 0; ChunkIndex < Chunks.Num(); ChunkIndex++)
	{
		ABoxelChunk* Chunk = Chunks[ChunkIndex];
		if (!Chunk) continue;
		
		Chunk->GenerateMesh();
	}
}

void ABoxelWorld::ClearChunks()
{
	for (int i = 0; i < Chunks.Num(); i++)
	{
		ABoxelChunk* Chunk = Chunks[i];
		if (!Chunk) continue;
		
		if (UProceduralMeshComponent* MeshComp = Chunk->GetMesh())
		{
			MeshComp->ClearAllMeshSections();
		}
		
		Chunk->Destroy();
	}
	
	Chunks.Empty();
}