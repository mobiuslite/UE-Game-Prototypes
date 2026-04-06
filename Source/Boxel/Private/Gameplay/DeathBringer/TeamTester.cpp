// 


#include "Gameplay/DeathBringer/TeamTester.h"

#include "Gameplay/Player/BoxelPlayerState.h"
#include "Net/UnrealNetwork.h"


ATeamTester::ATeamTester()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ATeamTester::AddPawnToEvaluator(const APawn* Pawn)
{
	if (!HasAuthority()) return;
	if (PlayersToEvaluate.Num() == NumEvaluators) return;
	if (!Pawn) return;
	if (!IsPowered()) return;
	
	if (ABoxelPlayerState* PlayerState = Pawn->GetPlayerState<ABoxelPlayerState>())
	{
		if (PlayersToEvaluate.Contains(PlayerState)) return;
		
		PlayersToEvaluate.Add(PlayerState);
		OnEvaluatorAdded.Broadcast(Pawn);
	}
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
	return !bRequiresPower || bPowered;
}

void ATeamTester::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bPowered);
}

void ATeamTester::OnRep_Powered_Implementation()
{
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


