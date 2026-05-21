// 


#include "Gameplay/DeathBringer/PowerSwitch.h"

#include "Core/MobiusGameState.h"
#include "Gameplay/DeathBringer/TeamTester.h"
#include "Net/UnrealNetwork.h"
#include "Utility/MobiusUtils.h"

APowerSwitch::APowerSwitch()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void APowerSwitch::SetPowered(const bool bPower)
{
	bPowered = bPower;
	OnRep_Powered();
}

void APowerSwitch::SetBroken(const bool bBroke)
{
	bBroken = bBroke;
}

void APowerSwitch::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bPowered);
	DOREPLIFETIME(ThisClass, bBroken);
}

void APowerSwitch::BeginPlay()
{
	Super::BeginPlay();
	
	if (ATeamTester* Tester = UMobiusUtils::GetActorOfClassEX<ATeamTester>(GetWorld(), ATeamTester::StaticClass()))
	{
		Tester->SetPowerSwitch(this);
	}
	
	if (AMobiusGameState* GameState = GetWorld()->GetGameState<AMobiusGameState>())
	{
		GameState->OnRoundHardResetDelegate.AddDynamic(this, &ThisClass::OnRoundReset);
	}
}

void APowerSwitch::OnRoundReset()
{
	SetPowered(false);
	BP_OnRoundReset();
}

void APowerSwitch::OnRep_Powered_Implementation()
{
	OnPoweredStateChanged.Broadcast(bPowered);
}

