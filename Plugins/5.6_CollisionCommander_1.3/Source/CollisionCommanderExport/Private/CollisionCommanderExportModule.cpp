// Copyright 2026 Paracosm. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Export module stub — xlnt integration and Excel export deferred to Phase 4.
class FCollisionCommanderExportModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FCollisionCommanderExportModule, CollisionCommanderExport)
