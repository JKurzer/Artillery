// Copyright Epic Games, Inc. All Rights Reserved.
using System;
using System.IO;
using EpicGames.Core;
using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class ArtilleryRuntime : ModuleRules
{
	public ArtilleryRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		//PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(PluginDirectory,"Source/ArtilleryRuntime")
			}
		);

				
		PublicIncludePaths.Add(
			Path.Combine(PluginDirectory,"Data")
		);
		
		RuntimeDependencies.Add(
			Path.Combine(PluginDirectory,"Data")
			);

		DirectoryReference m = DirectoryReference.FromString(Path.Combine(PluginDirectory, "Data"));
		if (m != null)
		{
			ConditionalAddModuleDirectory(m);
		}
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"ApplicationCore",
				"InputCore",
                "SlateCore",
				"GameplayAbilities",
				"Bristlecone"
				// ... add other public dependencies that you statically link with here ...
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"GameplayAbilities",
				"Bristlecone"
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
