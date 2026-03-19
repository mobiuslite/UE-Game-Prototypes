using UnrealBuildTool;

public class Boxel : ModuleRules
{
	public Boxel(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"ToolkitSteamworks", 
			"Steamworks", 
			"OnlineSubsystemUtils",
			"BoxelWorldGeneration",
			"MobiusAbilityComponent",
			"MobiusUISubsystem",
			"GameplayAbilities",
			"CommonUI"
		});

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG" });
		
		// Uncomment if you are using online features
		PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
		
		DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
		DynamicallyLoadedModuleNames.Add("SocketSubsystemSteamIP");
		DynamicallyLoadedModuleNames.Add("SteamSockets");
	}
}
