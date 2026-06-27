// Copyright 2026 Paracosm. All Rights Reserved.

#include "UCollisionCommanderSettings.h"


UCollisionCommanderSettings::UCollisionCommanderSettings()
{
    // Place settings under Edit → Project Settings → Plugins.
    CategoryName = TEXT("Plugins");

    // Pre-populate all rule IDs with their default enabled state.
    // If the user has toggled any rule in the editor and saved, those values
    // will override these defaults once the CDO is loaded from
    // Config/CollisionCommander.ini.
    ValidationRuleEnabled.Add(FName("CS_CHAN_BUDGET_WARN"),  true);
    ValidationRuleEnabled.Add(FName("CS_CHAN_BUDGET_CRIT"),  true);
    ValidationRuleEnabled.Add(FName("CS_CHAN_UNUSED"),       true);
    ValidationRuleEnabled.Add(FName("CS_PRESET_DUPLICATE"),  true);
    ValidationRuleEnabled.Add(FName("CS_PRESET_ALL_IGNORE"), true);
    ValidationRuleEnabled.Add(FName("CS_STATIC_IGNORED"),    true);
    ValidationRuleEnabled.Add(FName("CS_NAME_CONFLICT"),     true);
    ValidationRuleEnabled.Add(FName("CS_VISIBILITY_BLOCK"),  true);

    // Disabled by default — these rules fire on every overlapping or
    // asymmetric preset pair and produce too much noise to be useful in the
    // validation panel. The logic is preserved in FCCValidator for future use
    // in the Actor Inspector, where per-actor context makes them actionable.
    ValidationRuleEnabled.Add(FName("CS_OVERLAP_MISMATCH"),  false);
    ValidationRuleEnabled.Add(FName("CS_OVERLAP_EVENT"),     false);
}


// static
const TMap<FName, bool>& UCollisionCommanderSettings::GetEnabledRules()
{
    return GetDefault<UCollisionCommanderSettings>()->ValidationRuleEnabled;
}
