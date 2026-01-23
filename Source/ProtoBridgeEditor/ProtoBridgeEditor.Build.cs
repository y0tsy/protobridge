using UnrealBuildTool;

public class ProtoBridgeEditor : ModuleRules
{
	public ProtoBridgeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"DeveloperSettings",
				"Projects",
				"MessageLog",
				"EditorSubsystem"
			}
		);
	}
}