// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UCollisionCommanderSettings.generated.h"

// ============================================================================
//  UCollisionCommanderSettings
//
//  Plugin settings stored in Config/CollisionCommander.ini.
//  Appears under Edit → Project Settings → Plugins → Collision Commander.
//
//  ValidationRuleEnabled maps each of the 10 validation rule IDs to a bool.
//  Pass GetEnabledRules() directly to FCCValidator::RunAll() at the call site.
//  Rules absent from the map are treated as enabled (forward-compatible with
//  future rule additions — same logic as FCCValidator::IsRuleEnabled).
// ============================================================================

UCLASS(config = CollisionCommander, defaultconfig)
class COLLISIONCOMMANDER_API UCollisionCommanderSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UCollisionCommanderSettings();

    // UDeveloperSettings interface — places the settings under "Plugins" in
    // the Project Settings sidebar.
    virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

    // When true, a one-time GET request is made to paracosm.gg on editor startup
    // to check whether a newer version of Collision Commander is available.
    // If a newer version is found, a notice is shown in the Matrix tab toolbar.
    // Disable this if your studio restricts outbound network access from the editor.
    UPROPERTY(config, EditAnywhere, Category = "General",
              meta = (ToolTip = "Check paracosm.gg for a newer version on editor startup. Shows a notice in the Matrix tab if an update is available. Disable if your studio restricts editor network access."))
    bool bCheckForUpdates = true;

    // RuleId → enabled.  All 10 rules default to true.
    // Toggling a rule off suppresses it from FCCValidator::RunAll().
    // Adding a new rule ID here in a future version keeps existing .ini files
    // valid: unknown keys are preserved and new keys pick up the default.
    UPROPERTY(config, EditAnywhere, Category = "Validation Rules",
              meta = (ToolTip = "Enable or disable individual validation rules. Disabled rules are skipped during validation."))
    TMap<FName, bool> ValidationRuleEnabled;

    // Returns the current enabled-rules map from the CDO (i.e. the value
    // loaded from CollisionCommander.ini).
    // Pass the result directly to FCCValidator::RunAll().
    static const TMap<FName, bool>& GetEnabledRules();
};
