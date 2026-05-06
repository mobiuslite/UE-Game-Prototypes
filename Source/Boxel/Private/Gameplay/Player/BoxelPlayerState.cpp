

#include "Gameplay/Player/BoxelPlayerState.h"

#include "Core/MobiusGameState.h"
#include "Core/DeathBringer/DeathBringerGameMode.h"
#include "Gameplay/DeathBringer/Inventory/InventoryComponent.h"
#include "Gameplay/Player/BoxelPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Utility/MobiusUtils.h"


ABoxelPlayerState::ABoxelPlayerState()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

void ABoxelPlayerState::SetSpeaking(const bool bSpeaking, const bool bUseTeamChannel)
{
	if (bUseTeamChannel && !ADeathBringerGameMode::HasTeamVoiceChannel(GetTeamId().GetId()))
	{
		return;
	}
	
	OnSetSpeaking(bSpeaking, bUseTeamChannel);
}

void ABoxelPlayerState::OnSetSpeaking_Implementation(const bool bSpeaking, const bool bUseTeamChannel)
{
	OnSpeakingChanged.Broadcast(bSpeaking, bUseTeamChannel);
	if (const ABoxelPlayerController* BoxelController = Cast<ABoxelPlayerController>(GetPlayerController()))
	{
		BoxelController->OnSpeakingChanged.Broadcast(bSpeaking, bUseTeamChannel);
	}
	
	Server_SetIsTeamSpeaking(bUseTeamChannel && bSpeaking);
}

void ABoxelPlayerState::RegisterVoiceChannel(const uint8 ChannelID)
{
	RegisteredVoiceChannels.AddUnique(ChannelID);
}

void ABoxelPlayerState::UnregisterVoiceChannel(const uint8 ChannelID)
{
	RegisteredVoiceChannels.Remove(ChannelID);
}

void ABoxelPlayerState::UnregisterUncommonVoiceChannels()
{
	for (int i = 0; i < RegisteredVoiceChannels.Num();)
	{
		if (RegisteredVoiceChannels[i] > 1)
		{
			RegisteredVoiceChannels.RemoveAt(i);
		}
		else
		{
			i++;
		}
	}
}

bool ABoxelPlayerState::IsRegisteredToVoiceChannel(const uint8 ChannelID)
{
	return RegisteredVoiceChannels.Contains(ChannelID);
}

void ABoxelPlayerState::AUTH_SetTeamId(const FGenericTeamId& NewTeamID)
{
	if (HasAuthority())
	{
		SetGenericTeamId(NewTeamID);
	}
}

void ABoxelPlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	const FGenericTeamId OldTeamID = TeamID;
	TeamID = NewTeamID;
	OnRep_TeamID(OldTeamID);
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

void ABoxelPlayerState::SetTeammates_Implementation(const FGenericTeamId& Team,
	const TArray<ABoxelPlayerState*>& TeammatesPlayerStates)
{
	for (int i = 0; i < TeammatesPlayerStates.Num(); i++)
	{
		if (ABoxelPlayerState* State = TeammatesPlayerStates[i])
		{
			State->SetGenericTeamId(Team);
		}
	}
}

void ABoxelPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ThisClass, TeamID, COND_OwnerOnly);
}

void ABoxelPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (AMobiusGameState* GameState = GetWorld()->GetGameState<AMobiusGameState>())
	{
		GameState->OnRoundHardResetDelegate.AddDynamic(this, &ThisClass::OnRoundReset);
	}
	
	//Proximity voice channel
	RegisterVoiceChannel(EDeathBringerTeam::None);
	
	//Global voice channel (radio)
	RegisterVoiceChannel(EDeathBringerTeam::Normal);
}

void ABoxelPlayerState::OnLocalPlayerStateReady_Implementation()
{
	UE_LOG(LogTemp, Display, TEXT("Local player state ready!"))
}

void ABoxelPlayerState::OnProxyPlayerStateReady_Implementation()
{
	UE_LOG(LogTemp, Display, TEXT("Proxy player state ready!"))
}

void ABoxelPlayerState::Server_SetIsTeamSpeaking_Implementation(const bool bTeamSpeaking)
{
	const int32 NumPlayerStates = UGameplayStatics::GetNumPlayerStates(GetWorld());

	for (int i = 0; i < NumPlayerStates; ++i)
	{
		ABoxelPlayerState* OtherState = Cast<ABoxelPlayerState>(UGameplayStatics::GetPlayerState(GetWorld(), i));
		if (!OtherState) continue;
		if (OtherState == this) continue;
		if (GetTeamId() != OtherState->GetTeamId()) continue;
		
		OtherState->Client_SetTeammateIsSpeaking(bTeamSpeaking, this);
	}
}

void ABoxelPlayerState::Client_SetTeammateIsSpeaking_Implementation(const bool bTeamSpeaking, const APlayerState* PlayerState)
{
	if (!PlayerState) return;
	
	const uint64 UserID = FCString::Atoi64(*PlayerState->GetUniqueId()->ToString());
	BP_SetTeammateSpeaking(bTeamSpeaking, UserID, PlayerState);
}

void ABoxelPlayerState::OnRoundReset()
{
	SetGenericTeamId(FGenericTeamId::NoTeam);
	SetSpeaking(false, false);
			
	if (HasAuthority())
	{
		UnregisterVoiceChannel(DEAD_CHANNELID);
		
		UInventoryComponent* Inventory;
		if (UMobiusUtils::GetInventory(this, Inventory))
		{
			Inventory->ClearInventory();
		}
			
		if (UMobiusAbilitySystemComponent* AbilityComp = Cast<UMobiusAbilitySystemComponent>(GetAbilitySystemComponent()))
		{
			AbilityComp->ResetAttributes();
			AbilityComp->ClearAllAbilities();
		}
	}
}

void ABoxelPlayerState::OnRep_TeamID(const FGenericTeamId OldTeamID)
{
	ConditionalBroadcastTeamChanged(this, OldTeamID, TeamID);
}
