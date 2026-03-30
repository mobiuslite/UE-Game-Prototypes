#pragma once

#include "GenericTeamAgentInterface.h"
#include "UObject/Object.h"

#include "MLTeamInterface.generated.h"

template <typename InterfaceType> class TScriptInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTeamIndexChangedDelegate, UObject*, ObjectChangingTeam, int32, OldTeamID, int32, NewTeamID);

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UMLTeamAgentInterface : public UGenericTeamAgentInterface
{
	GENERATED_UINTERFACE_BODY()
};

class BOXEL_API IMLTeamAgentInterface : public IGenericTeamAgentInterface
{
	GENERATED_IINTERFACE_BODY()

	virtual FOnTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }

	static void ConditionalBroadcastTeamChanged(const TScriptInterface<IMLTeamAgentInterface>& This, const FGenericTeamId OldTeamID, const FGenericTeamId NewTeamID);
	
	FOnTeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
	{
		FOnTeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();
		check(Result);
		return *Result;
	}
};
