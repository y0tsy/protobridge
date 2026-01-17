using UnrealBuildTool;
using System.IO; 

public class ProtoBridgeCore : ModuleRules
{
	public ProtoBridgeCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		bUseRTTI = true;
		bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "ProtoBridgeThirdParty", "GameplayTags", "Json" });
		PrivateDependencyModuleNames.AddRange(new string[] { "CoreUObject", "Engine" }); 
	}
}