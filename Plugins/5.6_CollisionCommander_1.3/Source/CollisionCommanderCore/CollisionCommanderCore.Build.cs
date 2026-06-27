// Copyright 2026 Paracosm. All Rights Reserved.

using UnrealBuildTool;

// Core has no Slate or UnrealEd dependencies — it must stay testable in isolation.
public class CollisionCommanderCore : ModuleRules
{
    public CollisionCommanderCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",       // UCollisionProfile lives here
        });
    }
}
