#include "BoxelDebugModule.h"

#include "GameplayDebugger.h"
#include "GameplayDebuggerCategory_Boxel.h"

#define LOCTEXT_NAMESPACE "FBoxelDebugModuleModule"

void FBoxelDebugModuleModule::StartupModule()
{
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory("BoxelDebugger",
		IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_Boxel::MakeInstance),
		EGameplayDebuggerCategoryState::EnabledInGameAndSimulate, 3);
	GameplayDebuggerModule.NotifyCategoriesChanged();
}

void FBoxelDebugModuleModule::ShutdownModule()
{
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory("BoxelDebugger");
		GameplayDebuggerModule.NotifyCategoriesChanged();
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FBoxelDebugModuleModule, BoxelDebugModule)