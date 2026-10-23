using UnrealBuildTool;

public class RicoveroEditorTarget : TargetRules
{
	public RicoveroEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Ricovero");
	}
}
