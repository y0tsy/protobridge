using System;
using System.IO;
using System.Collections.Generic;
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
            PublicDefinitions.Add("PROTOBUF_USE_DLLS=1");
            PublicDefinitions.Add("GRPC_DLL_IMPORT=1");

            string PlatformLib = Path.Combine(LibPath, "Win64");
            string PlatformBin = Path.Combine(BinPath, "Win64");

            string[] LibNames = 
            { 
                "libprotobuf", 
                "libprotoc", 
                "gpr", 
                "grpc", 
                "grpc++",
                "grpc++_reflection",
                "address_sorting",
                "upb",
                "cares",
                "zlib"
            };

            foreach (string LibName in LibNames)
            {
                AddDynamicLibrary(PlatformLib, PlatformBin, LibName, ".lib", ".dll");
            }

            AddAbseilLibs(PlatformLib);
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string PlatformLib = Path.Combine(LibPath, "Linux");
            string PlatformBin = Path.Combine(BinPath, "Linux");
            
            string[] LibNames = 
            { 
                "libprotobuf", 
                "libgpr", 
                "libgrpc", 
                "libgrpc++" 
            };

            foreach (string LibName in LibNames)
            {
                AddDynamicLibrary(PlatformLib, PlatformBin, LibName, ".so", ".so");
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string PlatformLib = Path.Combine(LibPath, "Mac");
            string PlatformBin = Path.Combine(BinPath, "Mac");

            string[] LibNames = 
            { 
                "libprotobuf", 
                "libgrpc", 
                "libgrpc++" 
            };

            foreach (string LibName in LibNames)
            {
                AddDynamicLibrary(PlatformLib, PlatformBin, LibName, ".dylib", ".dylib");
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string PlatformLib = Path.Combine(LibPath, "Android", "arm64-v8a");
            
            PublicAdditionalLibraries.Add(Path.Combine(PlatformLib, "libprotobuf.so"));
            PublicAdditionalLibraries.Add(Path.Combine(PlatformLib, "libgrpc++.so"));
            PublicAdditionalLibraries.Add(Path.Combine(PlatformLib, "libgrpc.so"));
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            string FrameworkPath = Path.Combine(LibPath, "IOS");
            PublicAdditionalFrameworks.Add(
                new Framework(
                    "gRPC",
                    Path.Combine(FrameworkPath, "grpc.framework.zip")
                )
            );
             PublicAdditionalFrameworks.Add(
                new Framework(
                    "Protobuf",
                    Path.Combine(FrameworkPath, "protobuf.framework.zip")
                )
            );
        }
        else
        {
            throw new BuildException("ProtoBridgeThirdParty does not support this platform: " + Target.Platform.ToString());
        }
    }

    private void AddDynamicLibrary(string LibPath, string BinPath, string LibName, string LibExt, string DllExt)
    {
        string LibFile = Path.Combine(LibPath, LibName + LibExt);
        string DllFile = Path.Combine(BinPath, LibName + DllExt);

        if (File.Exists(LibFile))
        {
            PublicAdditionalLibraries.Add(LibFile);
        }

        if (File.Exists(DllFile))
        {
            RuntimeDependencies.Add(DllFile);
            
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                PublicDelayLoadDLLs.Add(LibName + DllExt);
            }
        }
    }

    private void AddAbseilLibs(string LibPath)
    {
        if (Directory.Exists(LibPath))
        {
            string[] Files = Directory.GetFiles(LibPath, "absl_*.lib");
            foreach(string FilePath in Files)
            {
                 PublicAdditionalLibraries.Add(FilePath);
            }
        }
    }
}