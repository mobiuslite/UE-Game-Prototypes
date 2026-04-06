

#include "Gameplay/Player/BoxelPlayerState.h"

#include "Gameplay/DeathBringer/InventoryComponent.h"
#include "Gameplay/Player/BoxelPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABoxelPlayerState::OnSetSpeaking_Implementation(const bool bSpeaking)
{
	OnSpeakingChanged.Broadcast(bSpeaking);
	if (const ABoxelPlayerController* BoxelController = Cast<ABoxelPlayerController>(GetPlayerController()))
	{
		BoxelController->OnSpeakingChanged.Broadcast(bSpeaking);
	}
}

ABoxelPlayerState::ABoxelPlayerState()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

void ABoxelPlayerState::SetSpeaking(const bool bSpeaking)
{
	OnSetSpeaking(bSpeaking);
}

void ABoxelPlayerState::AUTH_SetTeamId(const FGenericTeamId& NewTeamID)
{
	SetGenericTeamId(NewTeamID);
}

void ABoxelPlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (HasAuthority())
	{
		const FGenericTeamId OldTeamID = TeamID;
		TeamID = NewTeamID;
		OnRep_TeamID(OldTeamID);
	}
}

FGenericTeamId ABoxelPlayerState::GetGenericTeamId() const
{
	return TeamID;
}

void ABoxelPlayerState::BroadcastGunEquipped(const AGunBase* GunBase) const
{
	OnGunEquippedDelegate.Broadcast(GunBase);
}

void ABoxelPlayerState::BroadcastGunUnequipped(const AGunBase* GunBase) const
{
	OnGunUnequippedDelegate.Broadcast(GunBase);
}

UInventoryComponent* ABoxelPlayerState::GetInventory() const
{
	return InventoryComponent;
}

void ABoxelPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ThisClass, TeamID, COND_OwnerOnly);
}

void ABoxelPlayerState::OnRep_TeamID(const FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, TeamID);
}
