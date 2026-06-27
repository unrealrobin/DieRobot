// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

// Core module — data reading and processing only. No Slate, no UI.
class FCollisionCommanderCoreModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
