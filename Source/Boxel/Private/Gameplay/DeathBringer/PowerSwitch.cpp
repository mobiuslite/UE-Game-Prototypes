// 


#include "Gameplay/DeathBringer/PowerSwitch.h"

#include "Core/MobiusGameMode.h"
#include "Gameplay/DeathBringer/TeamTester.h"
#include "Net/UnrealNetwork.h"
#include "Utility/MobiusUtils.h"

APowerSwitch::APowerSwitch()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void APowerSwitch::SetPowered(const bool bPower)
{
	bPowered = bPower;
	OnRep_Powered();
}

void APowerSwitch::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bPowered);
}

void APowerSwitch::BeginPlay()
{
	Super::BeginPlay();
	
	if (PowerSwitchID > -1)
	{
		const TArray<ATeamTester*> Testers = UMobiusUtils::GetAllActorsOfClassEX<ATeamTester>(GetWorld(), ATeamTester::StaticClass());
		for (int i = 0; i < Testers.Num(); ++i)
		{
			ATeamTester* Tester = Testers[i];
			if (!Tester) continue;
		
			if (Tester->GetPowerSwitchID() == PowerSwitchID)
			{
				Tester->SetPowerSwitch(this);
			}
		}
	}
	
	if (AMobiusGameMode* GameMode = GetWorld()->GetAuthGameMode<AMobiusGameMode>())
	{
		GameMode->OnRoundHardResetDelegate.AddDynamic(this, &ThisClass::AUTH_OnRoundReset);
	}
}

void APowerSwitch::AUTH_OnRoundReset()
{
	SetPowered(false);
}

void APowerSwitch::OnRep_Powered_Implementation()
{
}

