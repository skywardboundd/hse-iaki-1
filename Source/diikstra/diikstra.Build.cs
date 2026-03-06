// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class diikstra : ModuleRules
{
	public diikstra(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"ProceduralMeshComponent"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"diikstra",
			"diikstra/Variant_Platforming",
			"diikstra/Variant_Platforming/Animation",
			"diikstra/Variant_Combat",
			"diikstra/Variant_Combat/AI",
			"diikstra/Variant_Combat/Animation",
			"diikstra/Variant_Combat/Gameplay",
			"diikstra/Variant_Combat/Interfaces",
			"diikstra/Variant_Combat/UI",
			"diikstra/Variant_SideScrolling",
			"diikstra/Variant_SideScrolling/AI",
			"diikstra/Variant_SideScrolling/Gameplay",
			"diikstra/Variant_SideScrolling/Interfaces",
			"diikstra/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
