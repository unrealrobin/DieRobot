// Copyright 2026 Paracosm. All Rights Reserved.

using UnrealBuildTool;

public class CollisionCommanderTests : ModuleRules
{
    public CollisionCommanderTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "CollisionCommanderCore",
        });
    }
}
