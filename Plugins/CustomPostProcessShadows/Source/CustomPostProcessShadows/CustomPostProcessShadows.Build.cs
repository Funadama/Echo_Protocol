// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class CustomPostProcessShadows : ModuleRules
{
    public CustomPostProcessShadows(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
                Path.Combine(GetModuleDirectory("Engine"), "Public"),
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
				// ... add other private include paths required here ...
                Path.Combine(GetModuleDirectory("Renderer"), "Private"),
                Path.Combine(GetModuleDirectory("Renderer"), "Internal"),
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
                "RenderCore",
                "Renderer",
                "RHI",
                "Projects"
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
