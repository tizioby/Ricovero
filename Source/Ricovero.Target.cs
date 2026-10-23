using UnrealBuildTool;

public class RicoveroTarget : TargetRules
{
	public RicoveroTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Ricovero");
	}
}
