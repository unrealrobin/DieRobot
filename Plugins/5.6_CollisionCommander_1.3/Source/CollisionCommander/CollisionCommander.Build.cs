// Copyright 2026 Paracosm. All Rights Reserved.

using UnrealBuildTool;

public class CollisionCommander : ModuleRules
{
    public CollisionCommander(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "EditorStyle",
            "EditorWidgets",
            "UnrealEd",
            "ToolMenus",
            "WorkspaceMenuStructure",
            "InputCore",
            "PropertyEditor",
            "DeveloperSettings",
            "CollisionCommanderCore",
            "Projects",   // IPluginManager
            "Http",        // FHttpModule — version check request
            "Json",        // FJsonSerializer — version check response parsing
        });
    }
}
