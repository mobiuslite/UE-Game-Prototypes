// 


#include "Core/DeathBringer/DeathBringerGameState.h"

#include "Net/UnrealNetwork.h"

void ADeathBringerGameState::SetRoundState(const EDeathBringerRoundState State)
{
	if (!HasAuthority()) return;
	RoundState = State;
}

void ADeathBringerGameState::SetRoundEndTime(const float EndTime)
{
	if (!HasAuthority()) return;
	RoundEndTimeWorldSeconds = EndTime + GetServerWorldTimeSeconds();
}

void ADeathBringerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, RoundState);
	DOREPLIFETIME(ThisClass, RoundEndTimeWorldSeconds);
}
