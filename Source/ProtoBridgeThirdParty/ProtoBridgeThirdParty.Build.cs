using System;
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
            string PlatformBinDir = Path.Combine(BinPath, "Win64");
            if (Directory.Exists(PlatformBinDir))
            {
                string[] BinFiles = Directory.GetFiles(PlatformBinDir, "*.dll", SearchOption.AllDirectories);
                foreach (string BinFile in BinFiles)
                {
                    RuntimeDependencies.Add(BinFile);
                    string FileName = Path.GetFileName(BinFile);
                    PublicDelayLoadDLLs.Add(FileName);
                }
            }
            
            string PlatformLibDir = Path.Combine(LibPath, "Win64");
            if (Directory.Exists(PlatformLibDir))
            {
                string[] LibFiles = Directory.GetFiles(PlatformLibDir, "*.lib", SearchOption.AllDirectories);
                foreach (string LibFile in LibFiles)
                {
                    PublicAdditionalLibraries.Add(LibFile);
                }
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string PlatformLibDir = Path.Combine(LibPath, "Linux");
            string PlatformBinDir = Path.Combine(BinPath, "Linux");

            if (Directory.Exists(PlatformLibDir))
            {
                string[] LibFiles = Directory.GetFiles(PlatformLibDir, "*.so*", SearchOption.AllDirectories);
                foreach (string LibFile in LibFiles)
                {
                    PublicAdditionalLibraries.Add(LibFile);
                    
                    string FileName = Path.GetFileName(LibFile);
                    string BinFile = Path.Combine(PlatformBinDir, FileName);
                    if (File.Exists(BinFile))
                    {
                        RuntimeDependencies.Add(BinFile);
                    }
                }
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string PlatformLibDir = Path.Combine(LibPath, "Android", "arm64-v8a");
            if (Directory.Exists(PlatformLibDir))
            {
                string[] LibFiles = Directory.GetFiles(PlatformLibDir, "*.so", SearchOption.AllDirectories);
                foreach (string LibFile in LibFiles)
                {
                    PublicAdditionalLibraries.Add(LibFile);
                }
            }
        }
    }
}