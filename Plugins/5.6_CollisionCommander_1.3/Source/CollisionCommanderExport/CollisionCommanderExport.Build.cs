// Copyright 2026 Paracosm. All Rights Reserved.

using UnrealBuildTool;

// Export module — xlnt integration deferred to Phase 4. Stub only.
public class CollisionCommanderExport : ModuleRules
{
    public CollisionCommanderExport(ReadOnlyTargetRules Target) : base(Target)
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
