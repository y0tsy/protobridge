using System.IO;
using UnrealBuildTool;

public class ProtoBridgeThirdParty : ModuleRules
{
	public ProtoBridgeThirdParty(ReadOnlyTargetRules Target) : base(Target)
	{
		// Define this module as "External". 
		// External modules do not contain C++ source code to be compiled by Unreal Build Tool,
		// but instead provide pre-compiled headers and libraries for other modules to consume.
		Type = ModuleType.External;

		// Enable C++ exception handling and Run-Time Type Information (RTTI).
		// These are often required when linking against standard third-party C++ libraries.
		bEnableExceptions = true;
		bUseRTTI = true;
		
		string ThirdPartyPath = ModuleDirectory;
		string IncludePath = Path.Combine(ThirdPartyPath, "includes");
		string LibBasePath = Path.Combine(ThirdPartyPath, "lib");

		// Add the include directory to the system include paths.
		// This ensures that any module depending on this one can include the third-party headers.
		PublicSystemIncludePaths.Add(IncludePath);

		// Configure linking settings based on the target platform.
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string PlatformLibPath = Path.Combine(LibBasePath, "Win64");
			AddLibrariesFromDirectory(PlatformLibPath, "*.lib");

			// Handle dynamic libraries (DLLs).
			string BinPath = Path.Combine(ThirdPartyPath, "bin", "Win64");
			string ZlibDllPath = Path.Combine(BinPath, "zlib.dll");
			
			// If the DLL exists, ensure it is copied to the binary directory and delay-loaded.
			if (File.Exists(ZlibDllPath))
			{
				// Ensure the DLL is copied to the build output directory so the executable can find it.
				RuntimeDependencies.Add(ZlibDllPath);
				
				// Mark the DLL for delay loading, allowing the engine to load it explicitly at runtime 
				// rather than strictly at startup.
				PublicDelayLoadDLLs.Add("zlib.dll");
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			string PlatformLibPath = Path.Combine(LibBasePath, "Linux");
			AddLibrariesFromDirectory(PlatformLibPath, "*.a");
			
			// Link specific system libraries required by the third-party code on Linux:
			// z (zlib), dl (dynamic linking), pthread (POSIX threads).
			PublicSystemLibraries.AddRange(new string[] { "z", "dl", "pthread" });
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			string PlatformLibPath = Path.Combine(LibBasePath, "Mac");
			AddLibrariesFromDirectory(PlatformLibPath, "*.a");

			// Link system libraries and macOS specific frameworks.
			PublicSystemLibraries.Add("z");
			PublicFrameworks.AddRange(new string[] { "CoreFoundation", "Security" });
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			string PlatformLibPath = Path.Combine(LibBasePath, "IOS");
			AddLibrariesFromDirectory(PlatformLibPath, "*.a");

			// Link system libraries and iOS specific frameworks.
			PublicSystemLibraries.Add("z");
			PublicFrameworks.AddRange(new string[] { "CoreFoundation", "Security" });
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string PlatformLibPath = Path.Combine(LibBasePath, "Android", "arm64-v8a");
			AddLibrariesFromDirectory(PlatformLibPath, "*.a");

			// Link system libraries required on Android:
			// z (zlib), log (Android logging), dl (dynamic linking).
			PublicSystemLibraries.AddRange(new string[] { "z", "log", "dl" });
		}
	}

	/// <summary>
	/// Helper method to scan a specific directory for library files matching a pattern
	/// and add them to the PublicAdditionalLibraries list.
	/// </summary>
	/// <param name="DirectoryPath">The absolute path to the directory containing libraries.</param>
	/// <param name="SearchPattern">The file pattern to match (e.g., "*.lib" or "*.a").</param>
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