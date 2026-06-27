// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FCollisionCommanderModule : public IModuleInterface
{
public:
    // Tab identifier — exposed so Task 8's matrix-click handler can invoke the tab
    // and switch to Actor Compare without a direct dependency on this module's internals.
    static const FName TabId;

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    // Returns the latest available version string fetched from paracosm.gg.
    // Empty if no update is available or the check has not completed yet.
    // Read by SCollisionMatrixPanel to drive the update notice widget.
    static const FString& GetAvailableVersion();

private:
    void RegisterMenus();
    static void OpenCollisionCommanderTab();
};
