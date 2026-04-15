// 


#include "Core/DeathBringer/DeathBringerGameState.h"

#include "GenericTeamAgentInterface.h"
#include "Net/UnrealNetwork.h"

void ADeathBringerGameState::SetRoundState(const EDeathBringerRoundState::Type State)
{
	if (!HasAuthority()) return;
	RoundState = State;
}

void ADeathBringerGameState::SetRoundEndTime(const float EndTime)
{
	if (!HasAuthority()) return;
	RoundEndTimeWorldSeconds = EndTime + GetServerWorldTimeSeconds();
}

void ADeathBringerGameState::ShowRoundEndToast_Implementation(const bool bDeathBringerWin)
{
	BP_ShowRoundEndToast(bDeathBringerWin);
}

void ADeathBringerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, RoundState);
	DOREPLIFETIME(ThisClass, RoundEndTimeWorldSeconds);
}

void ADeathBringerGameState::BeginPlay()
{
	Super::BeginPlay();
	
	FGenericTeamId::SetAttitudeSolver(DeathBringerAttitudeSolver);
}

ETeamAttitude::Type ADeathBringerGameState::DeathBringerAttitudeSolver(FGenericTeamId A, FGenericTeamId B)
{
	ETeamAttitude::Type Result = ETeamAttitude::Friendly;
	if (A == B) return Result;
		
	switch (A.GetId())
	{
		//Every one is friendly to normals and saviours except deathbringers
	case EDeathBringerTeam::Normal:
	case EDeathBringerTeam::Saviour:
		{
			if (B == EDeathBringerTeam::DeathBringer)
			{
				Result = ETeamAttitude::Hostile;
			}
		}
		//Every one is hostile to deathbringers except themselves
	case EDeathBringerTeam::DeathBringer:
		{
			Result = ETeamAttitude::Hostile;
		}
	}
		
	return Result;
}
