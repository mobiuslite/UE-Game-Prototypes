
#include "Gameplay/Player/Team/MLTeamInterface.h"

#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MLTeamInterface)

UMLTeamAgentInterface::UMLTeamAgentInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void IMLTeamAgentInterface::ConditionalBroadcastTeamChanged(const TScriptInterface<IMLTeamAgentInterface>& This, const FGenericTeamId OldTeamID, const FGenericTeamId NewTeamID)
{
	if (OldTeamID != NewTeamID)
	{
		const int32 OldTeamIndex = OldTeamID.GetId(); 
		const int32 NewTeamIndex = NewTeamID.GetId();

		UObject* ThisObj = This.GetObject();

		This.GetInterface()->GetTeamChangedDelegateChecked().Broadcast(ThisObj, OldTeamIndex, NewTeamIndex);
	}
}

