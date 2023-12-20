// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class DatadogMod : ModuleRules
{
	public DatadogMod(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
			}
            );


        PrivateIncludePaths.AddRange(
            new string[] {
				// ... add other private include paths required here ...
			}
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "FactoryGame",
                "HTTP", "Json", "JsonUtilities",
                "zlib",
				// ... add private dependencies that you statically link with here ...	
			}
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );

        var PlatformName = Target.Platform.ToString();
        var ThirdPartyFolder = Path.Combine(ModuleDirectory, "../../ThirdParty");
        var LibraryFolder = Path.Combine(ThirdPartyFolder, PlatformName);
        //PublicIncludePaths.Add(Path.Combine(ThirdPartyFolder, "include"));

        //// zlib        
        //if (Target.Platform == UnrealTargetPlatform.Win64)
        //{
        //    PublicAdditionalLibraries.Add(Path.Combine(LibraryFolder, "zlibwapi.lib"));
        //} else if (Target.Platform == UnrealTargetPlatform.Linux)
        //{
        //    PublicAdditionalLibraries.Add(Path.Combine(LibraryFolder, "libz.a"));
        //}
    }
}
