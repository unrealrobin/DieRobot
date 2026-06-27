// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DieRobot : ModuleRules
{
	public DieRobot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"EnhancedInput", 
			"UMG", 
			"SlateCore", 
			"Slate", 
			"Niagara", 
			"MetasoundEngine", 
			"GameplayTags",
			"OnlineServicesInterface", 
			"CoreOnline",
			"OnlineServicesCommon",
			"OnlineServicesEOS",
			"NavigationSystem",
			"Sentry",
			"ModelViewViewModel"
		});
	}
}