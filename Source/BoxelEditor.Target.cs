using UnrealBuildTool;
using System.Collections.Generic;

public class BoxelEditorTarget : TargetRules
{
	public BoxelEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("Boxel");
	}
}
