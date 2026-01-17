using System;
using System.IO;
using UnrealBuildTool;

public class ProtoBridgeThirdParty : ModuleRules
{
    public ProtoBridgeThirdParty(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        PublicDefinitions.Add("WITH_GRPC=1");

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
                string[] LibFiles = Directory.GetFiles(PlatformLibDir, "*.lib");
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

            string[] SharedLibs = 
            { 
                "libprotobuf.so", 
                "libgpr.so", 
                "libgrpc.so", 
                "libgrpc++.so" 
            };

            foreach (string LibName in SharedLibs)
            {
                string LibFile = Path.Combine(PlatformLibDir, LibName);
                string BinFile = Path.Combine(PlatformBinDir, LibName);

                if (File.Exists(LibFile))
                {
                    PublicAdditionalLibraries.Add(LibFile);
                    RuntimeDependencies.Add(BinFile);
                }
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string PlatformLibDir = Path.Combine(LibPath, "Android", "arm64-v8a");
            
            string[] AndroidLibs = 
            { 
                "libprotobuf.so", 
                "libgrpc++.so", 
                "libgrpc.so" 
            };

            foreach (string LibName in AndroidLibs)
            {
                string FullPath = Path.Combine(PlatformLibDir, LibName);
                if (File.Exists(FullPath))
                {
                    PublicAdditionalLibraries.Add(FullPath);
                }
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            string FrameworkPath = Path.Combine(LibPath, "IOS");
            PublicAdditionalFrameworks.Add(new Framework("gRPC", Path.Combine(FrameworkPath, "grpc.framework.zip")));
            PublicAdditionalFrameworks.Add(new Framework("Protobuf", Path.Combine(FrameworkPath, "protobuf.framework.zip")));
        }
    }
}