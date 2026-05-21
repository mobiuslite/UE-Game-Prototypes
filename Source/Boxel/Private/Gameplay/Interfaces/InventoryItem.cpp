// 


#include "Gameplay/Interfaces/InventoryItem.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "MobiusAbilitySystem/MobiusAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"


AInventoryItem::AInventoryItem()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);
}

bool AInventoryItem::CanBePickedUp(const APawn* PawnHolder) const
{
	return HolderHistory.Num() == 0;
}

void AInventoryItem::SetPhysicsEnabled(const bool bEnabled)
{
	SetActorEnableCollision(bEnabled);
	SetReplicatingMovement(bEnabled);
}

void AInventoryItem::OnRep_Holder()
{
	SetOwner(HolderPrivate);
	
	//Disable collision while holding
	SetPhysicsEnabled(HolderPrivate == nullptr);
	
	if (HolderPrivate)
	{
		const FAttachmentTransformRules Rules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
		if (const ACharacter* Character = Cast<ACharacter>(HolderPrivate))
		{
			AttachToComponent(Character->GetMesh(), Rules, FName("GunSocket"));
		}
		else
		{
			AttachToActor(HolderPrivate, Rules);	
		}
	}
	else
	{
		if (CanBeDropped())
		{
			const FDetachmentTransformRules Rules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, false); 
			DetachFromActor(Rules);
		}
		else
		{
			Destroy();
		}
	}
}

void AInventoryItem::OnRep_Visible()
{
	SetActorHiddenInGame(!bVisible);
}

void AInventoryItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, HolderPrivate);
	DOREPLIFETIME(ThisClass, bVisible);
	DOREPLIFETIME(ThisClass, bCanBeDropped);
}

void AInventoryItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	for (int i = 0; i < HolderHistory.Num();)
	{
		const FHolderHistoryData& History = HolderHistory[i];
		float Timer = History.HeldCooldownTimer;
		Timer -= DeltaSeconds;
		if (Timer <= 0.0f)
		{
			HolderHistory.RemoveAt(i);
			OnHolderHistoryRemoved();
		}
		else
		{
			FHolderHistoryData DataOverride;
			DataOverride.HeldCooldownTimer = Timer;
			DataOverride.PreviousHolder = History.PreviousHolder;
			HolderHistory[i] = DataOverride;
			i++;
		}
	}
}

void AInventoryItem::OnEquip_Implementation(AController* HolderController)
{
	if (UMobiusAbilitySystemComponent* AbilityComp = Cast<UMobiusAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HolderController->GetPlayerState<APlayerState>())))
	{
		if (HasAuthority() && IsValid(EquippedGrantedAbilityClass))
		{
			AbilityHandle = AbilityComp->K2_GiveAbility(EquippedGrantedAbilityClass, 0, 1, this);
		}
		
		if (IsValid(EquippedGrantedEffectClass))
		{
			const FGameplayEffectContextHandle Context = FGameplayEffectContextHandle(UAbilitySystemGlobals::Get().AllocGameplayEffectContext());
			
			const FGameplayEffectSpec	Spec(EquippedGrantedEffectClass->GetDefaultObject<UGameplayEffect>(), Context, 0.0f);
			EffectHandle = AbilityComp->ApplyGameplayEffectSpecToSelf(Spec);
		}
	}
	
	bVisible = true;
	OnRep_Visible();
}

void AInventoryItem::OnUnequip_Implementation(AController* HolderController)
{
	if (UAbilitySystemComponent* AbilityComp = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HolderController->GetPlayerState<APlayerState>()))
	{
		if (HasAuthority() && AbilityHandle.IsValid())
		{
			AbilityComp->ClearAbility(AbilityHandle);
			AbilityHandle = FGameplayAbilitySpecHandle();
		}
			
		if (EffectHandle.IsValid())
		{
			AbilityComp->RemoveActiveGameplayEffect(EffectHandle);
		}
	}
	
	bVisible = false;
	OnRep_Visible();
}

void AInventoryItem::OnAddedToInventory_Implementation(const UInventoryComponent* Inventory,
	AController* HolderController)
{
	HolderPrivate = HolderController->GetPawn();
	OnRep_Holder();
	
	bVisible = false;
	OnRep_Visible();
}

void AInventoryItem::OnRemovedFromInventory_Implementation(const UInventoryComponent* Inventory,
	AController* HolderController)
{
	if (HolderPrivate)
	{
		FHolderHistoryData History;
		History.PreviousHolder = HolderPrivate;
		History.HeldCooldownTimer = 0.5f;
		
		HolderHistory.Add(History);
		OnHolderHistoryAdded();
	}
	
	HolderPrivate = nullptr;
	OnRep_Holder();
	
	bVisible = true;
	OnRep_Visible();
}


