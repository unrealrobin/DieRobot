// Copyright 2026 Paracosm. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Test module — unit tests for CollisionCommanderCore processing layer only.
class FCollisionCommanderTestsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FCollisionCommanderTestsModule, CollisionCommanderTests)
