// 


#include "Gameplay/DeathBringer/TeamTester.h"

#include "Core/MobiusGameMode.h"
#include "Gameplay/DeathBringer/PowerSwitch.h"
#include "Gameplay/Player/BoxelPlayerState.h"
#include "Net/UnrealNetwork.h"


ATeamTester::ATeamTester()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

bool ATeamTester::AddPawnToEvaluator(const APawn* Pawn)
{
	if (!HasAuthority()) return false;
	if (PlayersToEvaluate.Num() == NumEvaluators) return false;
	if (!Pawn) return false;
	if (!IsPowered()) return false;
	
	if (ABoxelPlayerState* PlayerState = Pawn->GetPlayerState<ABoxelPlayerState>())
	{
		if (PlayersToEvaluate.Contains(PlayerState)) return false;
		
		PlayersToEvaluate.Add(PlayerState);
		OnEvaluatorAdded.Broadcast(Pawn);
	}
	
	return true;
}

bool ATeamTester::StartEvaluation()
{
	if (!HasAuthority()) return false;
	if (PlayersToEvaluate.Num() != NumEvaluators) return false;
	if (!IsPowered()) return false;
	
	DisplayResults(Evaluate());
	PlayersToEvaluate.Empty();
	
	return true;
}

bool ATeamTester::IsPowered() const
{
	if (!bRequiresPower) return true;
	
	if (!IsValid(ConnectedSwitch)) return false;
	return ConnectedSwitch->IsPowered();
}

void ATeamTester::SetPowerSwitch(const TObjectPtr<APowerSwitch> Switch)
{
	ConnectedSwitch = Switch;
}

void ATeamTester::BeginPlay()
{
	Super::BeginPlay();
	
	if (AMobiusGameMode* GameMode = GetWorld()->GetAuthGameMode<AMobiusGameMode>())
	{
		GameMode->OnRoundHardResetDelegate.AddDynamic(this, &ThisClass::AUTH_OnRoundReset);
	}
}

void ATeamTester::AUTH_OnRoundReset()
{
	PlayersToEvaluate.Empty();
}

void ATeamTester::DisplayResults_Implementation(const bool bEvaluatorsSafe)
{
	BP_DisplayResults(bEvaluatorsSafe);
	OnEvaluated.Broadcast(bEvaluatorsSafe);
}

bool ATeamTester::Evaluate_Implementation()
{
	return true;
}


