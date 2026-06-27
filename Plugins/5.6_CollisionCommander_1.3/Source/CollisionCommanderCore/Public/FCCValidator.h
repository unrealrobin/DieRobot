// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CCTypes.h"

// ============================================================================
//  FCCValidator
//
//  Runs the 10 Phase-4 validation rules against an FCCCollisionSnapshot and
//  returns a flat list of FCCValidationResult, grouped by the caller if needed.
//
//  All methods are stateless and pure — no editor context required.
//  Safe to call from unit tests without a live UCollisionProfile.
//
//  Rule IDs (FName keys):
//    CS_CHAN_BUDGET_WARN   CS_CHAN_BUDGET_CRIT   CS_CHAN_UNUSED
//    CS_PRESET_DUPLICATE  CS_PRESET_ALL_IGNORE  CS_STATIC_IGNORED
//    CS_OVERLAP_MISMATCH  CS_NAME_CONFLICT
//    CS_OVERLAP_EVENT     CS_VISIBILITY_BLOCK
// ============================================================================
struct COLLISIONCOMMANDERCORE_API FCCValidator
{
    // Runs all enabled rules against Snapshot.
    // EnabledRules maps RuleId -> enabled. Empty map = run all rules.
    // If a key is present with value false, that rule is skipped.
    static TArray<FCCValidationResult> RunAll(
        const FCCCollisionSnapshot& Snapshot,
        const TMap<FName, bool>&    EnabledRules = TMap<FName, bool>());

    // Runs component-level checks that require live actor data (not available in
    // FCCCollisionSnapshot which only holds UCollisionProfile preset data).
    // Call this separately and merge results with RunAll output if desired.
    static TArray<FCCValidationResult> RunComponentChecks(
        const TArray<FCCActorComponentData>& ActorData);

private:
    // CS_CHAN_BUDGET_WARN / CS_CHAN_BUDGET_CRIT
    // bWarn: emit CS_CHAN_BUDGET_WARN; bCrit: emit CS_CHAN_BUDGET_CRIT.
    static void CheckChanBudget(const FCCCollisionSnapshot& Snapshot,
                                TArray<FCCValidationResult>& Results,
                                bool bWarn, bool bCrit);

    // CS_CHAN_UNUSED — custom channel with no non-Ignore QueryResponse across all presets.
    static void CheckChanUnused(const FCCCollisionSnapshot& Snapshot,
                                TArray<FCCValidationResult>& Results);

    // CS_PRESET_DUPLICATE — two or more presets share identical ObjectTypeChannel + QueryResponses.
    static void CheckPresetDuplicate(const FCCCollisionSnapshot& Snapshot,
                                     TArray<FCCValidationResult>& Results);

    // CS_PRESET_ALL_IGNORE — preset ignores every channel (built-in "NoCollision" exempt).
    static void CheckPresetAllIgnore(const FCCCollisionSnapshot& Snapshot,
                                     TArray<FCCValidationResult>& Results);

    // CS_STATIC_IGNORED — non-WorldStatic-type preset has QueryResponse to WorldStatic == Ignore.
    static void CheckStaticIgnored(const FCCCollisionSnapshot& Snapshot,
                                   TArray<FCCValidationResult>& Results);

    // CS_OVERLAP_MISMATCH — one preset Blocks the other, but the other Ignores (result: Ignore).
    // One result per unordered pair.
    static void CheckOverlapMismatch(const FCCCollisionSnapshot& Snapshot,
                                     TArray<FCCValidationResult>& Results);

    // CS_NAME_CONFLICT — custom preset name exactly matches a UE built-in preset name.
    static void CheckNameConflict(const FCCCollisionSnapshot& Snapshot,
                                  TArray<FCCValidationResult>& Results);

    // CS_OVERLAP_EVENT — pair resolves to QueryResult == Overlap.
    // One result per unordered pair.
    static void CheckOverlapEvent(const FCCCollisionSnapshot& Snapshot,
                                  TArray<FCCValidationResult>& Results);

    // CS_VISIBILITY_BLOCK — preset blocks the Visibility trace channel.
    static void CheckVisibilityBlock(const FCCCollisionSnapshot& Snapshot,
                                     TArray<FCCValidationResult>& Results);

    // CS_COMPONENT_CUSTOM_PRESET — component whose collision profile is "Custom"
    // (i.e. the developer manually overrode responses without using a named preset).
    static void CheckComponentCustomPreset(const TArray<FCCActorComponentData>& ActorData,
                                           TArray<FCCValidationResult>& Results);
};
