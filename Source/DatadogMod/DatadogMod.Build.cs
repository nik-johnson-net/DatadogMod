// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DatadogMod : ModuleRules
{
	public DatadogMod(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // FactoryGame transitive dependencies
        // Not all of these are required, but including the extra ones saves you from having to add them later.
        PublicDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject",
            "Engine",
            "DeveloperSettings",
            "PhysicsCore",
            "InputCore",
            "OnlineSubsystem", "OnlineSubsystemNull", "OnlineSubsystemUtils",
            "SignificanceManager",
            "GeometryCollectionEngine",
            "ChaosVehiclesCore", "ChaosVehicles", "ChaosSolverEngine",
            "AnimGraphRuntime",
            "AkAudio",
            "AssetRegistry",
            "NavigationSystem",
            "ReplicationGraph",
            "AIModule",
            "GameplayTasks",
            "SlateCore", "Slate", "UMG",
            "RenderCore",
            "CinematicCamera",
            "Foliage",
            "Niagara",
            "EnhancedInput",
            "GameplayCameras",
            "TemplateSequence",
            "NetCore",
            "GameplayTags",
        });

        // FactoryGame plugins
        PublicDependencyModuleNames.AddRange(new[] {
            "AbstractInstance",
            "InstancedSplinesComponent",
            "SignificanceISPC"
        });

        // Header stubs
        PublicDependencyModuleNames.AddRange(new[] {
            "DummyHeaders",
        });

        if (Target.Type == TargetRules.TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(new string[] { "OnlineBlueprintSupport", "AnimGraph" });
        }
        PublicDependencyModuleNames.AddRange(new string[] { "FactoryGame", "SML" });

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
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
