#pragma once

#include "CoreMinimal.h"
#include "BoxelBlock.generated.h"

USTRUCT()
struct FBoxelBlock
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 BlockIndex;
	UPROPERTY()
	int32 ChunkIndex;
	
	int32 BlockType;
};
