using System.IO;
using UnrealBuildTool;

public class ProtoBridgeThirdParty : ModuleRules
{
	public ProtoBridgeThirdParty(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		bEnableExceptions = true;
		bUseRTTI = true;

		string ThirdPartyPath = ModuleDirectory;
		string IncludePath = Path.Combine(ThirdPartyPath, "includes");
		string LibBasePath = Path.Combine(ThirdPartyPath, "lib");

		PublicSystemIncludePaths.Add(IncludePath);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string PlatformLibPath = Path.Combine(LibBasePath, "Win64");
			AddLibrariesFromDirectory(PlatformLibPath, "*.lib");

			string BinPath = Path.Combine(ThirdPartyPath, "bin", "Win64");
			string ZlibDllPath = Path.Combine(BinPath, "zlib.dll");
			if (File.Exists(ZlibDllPath))
			{
				RuntimeDependencies.Add(ZlibDllPath);
				PublicDelayLoadDLLs.Add("zlib.dll");
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			string PlatformLibPath = Path.Combine(LibBasePath, "Linux");
			AddLibrariesFromDirectory(PlatformLibPath, "*.a");
			
			PublicSystemLibraries.AddRange(new string[] { "z", "dl", "pthread" });
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			string PlatformLibPath = Path.Combine(LibBasePath, "Mac");
			AddLibrariesFromDirectory(PlatformLibPath, "*.a");

			PublicSystemLibraries.Add("z");
			PublicFrameworks.AddRange(new string[] { "CoreFoundation", "Security" });
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			string PlatformLibPath = Path.Combine(LibBasePath, "IOS");
			AddLibrariesFromDirectory(PlatformLibPath, "*.a");

			PublicSystemLibraries.Add("z");
			PublicFrameworks.AddRange(new string[] { "CoreFoundation", "Security" });
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string PlatformLibPath = Path.Combine(LibBasePath, "Android", "arm64-v8a");
			AddLibrariesFromDirectory(PlatformLibPath, "*.a");

			PublicSystemLibraries.AddRange(new string[] { "z", "log", "dl" });
		}
	}

	private void AddLibrariesFromDirectory(string DirectoryPath, string SearchPattern)
	{
		if (Directory.Exists(DirectoryPath))
		{
			string[] Files = Directory.GetFiles(DirectoryPath, SearchPattern, SearchOption.TopDirectoryOnly);
			foreach (string FilePath in Files)
			{
				PublicAdditionalLibraries.Add(FilePath);
			}
		}
	}
}