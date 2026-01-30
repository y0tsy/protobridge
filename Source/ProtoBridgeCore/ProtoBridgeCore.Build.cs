using UnrealBuildTool;

public class ProtoBridgeCore : ModuleRules
{
	public ProtoBridgeCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"ProtoBridgeThirdParty", 
			"GameplayTags", 
			"Json" 
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] { 
			"CoreUObject", 
			"Engine",
			"DeveloperSettings"
		});
	}
}