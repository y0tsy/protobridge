using System.IO;
using UnrealBuildTool;

public class ProtoBridgeThirdParty : ModuleRules
{
    public ProtoBridgeThirdParty(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        PublicDefinitions.Add("WITH_GRPC=1");

        bEnableExceptions = true;
        bUseRTTI = true;

        string ThirdPartyPath = ModuleDirectory;
        string IncludePath = Path.Combine(ThirdPartyPath, "includes");
        string LibPath = Path.Combine(ThirdPartyPath, "lib");
        string BinPath = Path.Combine(ThirdPartyPath, "bin");

        PublicSystemIncludePaths.Add(IncludePath);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string PlatformLibDir = Path.Combine(LibPath, "Win64");
            if (Directory.Exists(PlatformLibDir))
            {
                foreach (string LibFile in Directory.GetFiles(PlatformLibDir, "*.lib", SearchOption.TopDirectoryOnly))
                {
                    PublicAdditionalLibraries.Add(LibFile);
                }
            }

            string ZlibDllPath = Path.Combine(BinPath, "Win64", "zlib.dll");
            if (File.Exists(ZlibDllPath))
            {
                RuntimeDependencies.Add(ZlibDllPath);
                PublicDelayLoadDLLs.Add("zlib.dll");
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string PlatformLibDir = Path.Combine(LibPath, "Linux");
            if (Directory.Exists(PlatformLibDir))
            {
                foreach (string LibFile in Directory.GetFiles(PlatformLibDir, "*.a", SearchOption.TopDirectoryOnly))
                {
                    PublicAdditionalLibraries.Add(LibFile);
                }
            }
            PublicSystemLibraries.AddRange(new string[] { "z", "dl", "pthread" });
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string PlatformLibDir = Path.Combine(LibPath, "Mac");
            if (Directory.Exists(PlatformLibDir))
            {
                foreach (string LibFile in Directory.GetFiles(PlatformLibDir, "*.a", SearchOption.TopDirectoryOnly))
                {
                    PublicAdditionalLibraries.Add(LibFile);
                }
            }
            PublicSystemLibraries.Add("z");
            PublicFrameworks.AddRange(new string[] { "CoreFoundation", "Security" });
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            string PlatformLibDir = Path.Combine(LibPath, "IOS");
            if (Directory.Exists(PlatformLibDir))
            {
                foreach (string LibFile in Directory.GetFiles(PlatformLibDir, "*.a", SearchOption.TopDirectoryOnly))
                {
                    PublicAdditionalLibraries.Add(LibFile);
                }
            }
            PublicSystemLibraries.Add("z");
            PublicFrameworks.AddRange(new string[] { "CoreFoundation", "Security" });
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string PlatformLibDir = Path.Combine(LibPath, "Android", "arm64-v8a");
            if (Directory.Exists(PlatformLibDir))
            {
                foreach (string LibFile in Directory.GetFiles(PlatformLibDir, "*.a", SearchOption.TopDirectoryOnly))
                {
                    PublicAdditionalLibraries.Add(LibFile);
                }
            }
            PublicSystemLibraries.AddRange(new string[] { "z", "log", "dl" });
        }
    }
}