// Copyright 2026 Paracosm. All Rights Reserved.

#include "FCCValidator.h"

// ---------------------------------------------------------------------------
//  Well-known channel names used by multiple rules.
//  These are UE built-in channel names — never hardcode their indices.
// ---------------------------------------------------------------------------
static const FName GWorldStatic(TEXT("WorldStatic"));
static const FName GVisibility(TEXT("Visibility"));
static const FName GNoCollisionPreset(TEXT("NoCollision"));


// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------

// Returns true if RuleId should run given EnabledRules.
// Empty map = all rules enabled (test/default behaviour).
static bool IsRuleEnabled(const FName& RuleId, const TMap<FName, bool>& EnabledRules)
{
    if (EnabledRules.IsEmpty()) return true;
    const bool* bEnabled = EnabledRules.Find(RuleId);
    // Key absent from map → treat as enabled (forward-compatible with new rules).
    return bEnabled == nullptr || *bEnabled;
}

// Convenience: build a minimal FCCValidationResult with required fields set.
static FCCValidationResult MakeResult(FName RuleId, ECCValidationSeverity Severity,
                                      FString Description, FString SuggestedFix)
{
    FCCValidationResult R;
    R.RuleId       = RuleId;
    R.Severity     = Severity;
    R.Description  = MoveTemp(Description);
    R.SuggestedFix = MoveTemp(SuggestedFix);
    return R;
}


// ---------------------------------------------------------------------------
//  RunAll
// ---------------------------------------------------------------------------
TArray<FCCValidationResult> FCCValidator::RunAll(
    const FCCCollisionSnapshot& Snapshot,
    const TMap<FName, bool>&    EnabledRules)
{
    TArray<FCCValidationResult> Results;

    const bool bWarn = IsRuleEnabled("CS_CHAN_BUDGET_WARN", EnabledRules);
    const bool bCrit = IsRuleEnabled("CS_CHAN_BUDGET_CRIT", EnabledRules);
    if (bWarn || bCrit)
        CheckChanBudget(Snapshot, Results, bWarn, bCrit);

    if (IsRuleEnabled("CS_CHAN_UNUSED",       EnabledRules)) CheckChanUnused(Snapshot, Results);
    if (IsRuleEnabled("CS_PRESET_DUPLICATE",  EnabledRules)) CheckPresetDuplicate(Snapshot, Results);
    if (IsRuleEnabled("CS_PRESET_ALL_IGNORE", EnabledRules)) CheckPresetAllIgnore(Snapshot, Results);
    if (IsRuleEnabled("CS_STATIC_IGNORED",    EnabledRules)) CheckStaticIgnored(Snapshot, Results);
    if (IsRuleEnabled("CS_OVERLAP_MISMATCH",  EnabledRules)) CheckOverlapMismatch(Snapshot, Results);
    if (IsRuleEnabled("CS_NAME_CONFLICT",     EnabledRules)) CheckNameConflict(Snapshot, Results);
    if (IsRuleEnabled("CS_OVERLAP_EVENT",     EnabledRules)) CheckOverlapEvent(Snapshot, Results);
    if (IsRuleEnabled("CS_VISIBILITY_BLOCK",  EnabledRules)) CheckVisibilityBlock(Snapshot, Results);

    return Results;
}


// ---------------------------------------------------------------------------
//  CS_CHAN_BUDGET_WARN / CS_CHAN_BUDGET_CRIT
//  UE provides 18 custom channel slots. Warn at 14+, error at 17+.
// ---------------------------------------------------------------------------
void FCCValidator::CheckChanBudget(const FCCCollisionSnapshot& Snapshot,
                                   TArray<FCCValidationResult>& Results,
                                   bool bWarn, bool bCrit)
{
    int32 CustomCount = 0;
    for (const FCCChannelInfo& Ch : Snapshot.Channels)
    {
        if (!Ch.bIsBuiltin) ++CustomCount;
    }

    // CRIT takes priority when both are enabled and threshold is met.
    // Both rules are still independently evaluated so disabling one still
    // lets the other fire at its threshold.
    if (bCrit && CustomCount >= 17)
    {
        FCCValidationResult R = MakeResult(
            "CS_CHAN_BUDGET_CRIT",
            ECCValidationSeverity::Error,
            FString::Printf(TEXT("%d of 18 custom channel slots in use"), CustomCount),
            TEXT("Consolidate channels — you are near the 18-slot limit"));
        Results.Add(MoveTemp(R));
    }
    else if (bWarn && CustomCount >= 14)
    {
        FCCValidationResult R = MakeResult(
            "CS_CHAN_BUDGET_WARN",
            ECCValidationSeverity::Warning,
            FString::Printf(TEXT("%d of 18 custom channel slots in use"), CustomCount),
            TEXT("Audit unused channels before adding new ones"));
        Results.Add(MoveTemp(R));
    }
}


// ---------------------------------------------------------------------------
//  CS_CHAN_UNUSED
//  Custom channel with no preset responding non-Ignore to it (Query).
// ---------------------------------------------------------------------------
void FCCValidator::CheckChanUnused(const FCCCollisionSnapshot& Snapshot,
                                   TArray<FCCValidationResult>& Results)
{
    for (const FCCChannelInfo& Ch : Snapshot.Channels)
    {
        if (Ch.bIsBuiltin) continue;

        bool bAnyNonIgnore = false;
        for (const FCCPresetData& P : Snapshot.Presets)
        {
            const ECollisionResponse* Resp = P.QueryResponses.Find(Ch.Name);
            if (Resp && *Resp != ECR_Ignore)
            {
                bAnyNonIgnore = true;
                break;
            }
        }

        if (!bAnyNonIgnore)
        {
            FCCValidationResult R = MakeResult(
                "CS_CHAN_UNUSED",
                ECCValidationSeverity::Info,
                FString::Printf(TEXT("Channel '%s' is defined but no preset responds to it"),
                                *Ch.Name.ToString()),
                TEXT("Remove or mark as reserved for future use"));
            R.AffectedChannels.Add(Ch.Name);
            Results.Add(MoveTemp(R));
        }
    }
}


// ---------------------------------------------------------------------------
//  CS_PRESET_DUPLICATE
//  Two or more presets share identical ObjectTypeChannel AND QueryResponses.
//  One result per unordered pair.
// ---------------------------------------------------------------------------
void FCCValidator::CheckPresetDuplicate(const FCCCollisionSnapshot& Snapshot,
                                        TArray<FCCValidationResult>& Results)
{
    const int32 N = Snapshot.Presets.Num();
    for (int32 i = 0; i < N; ++i)
    {
        for (int32 j = i + 1; j < N; ++j)
        {
            const FCCPresetData& A = Snapshot.Presets[i];
            const FCCPresetData& B = Snapshot.Presets[j];

            if (A.ObjectTypeChannel != B.ObjectTypeChannel) continue;
            if (A.QueryResponses.Num() != B.QueryResponses.Num()) continue;

            bool bIdentical = true;
            for (const TPair<FName, ECollisionResponse>& KV : A.QueryResponses)
            {
                const ECollisionResponse* BResp = B.QueryResponses.Find(KV.Key);
                if (!BResp || *BResp != KV.Value)
                {
                    bIdentical = false;
                    break;
                }
            }

            if (bIdentical)
            {
                FCCValidationResult R = MakeResult(
                    "CS_PRESET_DUPLICATE",
                    ECCValidationSeverity::Warning,
                    FString::Printf(TEXT("Presets '%s' and '%s' have identical ObjectType and responses"),
                                    *A.PresetName.ToString(), *B.PresetName.ToString()),
                    TEXT("Consolidate or document why both exist"));
                R.AffectedPresets = { A.PresetName, B.PresetName };
                Results.Add(MoveTemp(R));
            }
        }
    }
}


// ---------------------------------------------------------------------------
//  CS_PRESET_ALL_IGNORE
//  Preset ignores every channel in QueryResponses.
//  The built-in "NoCollision" preset is exempt — it is expected to ignore all.
// ---------------------------------------------------------------------------
void FCCValidator::CheckPresetAllIgnore(const FCCCollisionSnapshot& Snapshot,
                                        TArray<FCCValidationResult>& Results)
{
    for (const FCCPresetData& P : Snapshot.Presets)
    {
        if (P.PresetName == GNoCollisionPreset) continue;
        if (P.QueryResponses.IsEmpty()) continue;

        bool bAllIgnore = true;
        for (const TPair<FName, ECollisionResponse>& KV : P.QueryResponses)
        {
            if (KV.Value != ECR_Ignore)
            {
                bAllIgnore = false;
                break;
            }
        }

        if (bAllIgnore)
        {
            FCCValidationResult R = MakeResult(
                "CS_PRESET_ALL_IGNORE",
                ECCValidationSeverity::Warning,
                FString::Printf(TEXT("Preset '%s' responds Ignore to every channel"),
                                *P.PresetName.ToString()),
                TEXT("Likely misconfiguration — confirm intent or use NoCollision"));
            R.AffectedPresets.Add(P.PresetName);
            Results.Add(MoveTemp(R));
        }
    }
}


// ---------------------------------------------------------------------------
//  CS_STATIC_IGNORED
//  Non-WorldStatic-type preset has QueryResponse to WorldStatic == Ignore.
//  Dynamic actors ignoring WorldStatic will fall through static geometry.
// ---------------------------------------------------------------------------
void FCCValidator::CheckStaticIgnored(const FCCCollisionSnapshot& Snapshot,
                                      TArray<FCCValidationResult>& Results)
{
    for (const FCCPresetData& P : Snapshot.Presets)
    {
        // WorldStatic-type presets don't need to collide with their own channel.
        if (P.ObjectTypeChannel == GWorldStatic) continue;

        const ECollisionResponse* Resp = P.QueryResponses.Find(GWorldStatic);
        if (Resp && *Resp == ECR_Ignore)
        {
            FCCValidationResult R = MakeResult(
                "CS_STATIC_IGNORED",
                ECCValidationSeverity::Warning,
                FString::Printf(
                    TEXT("Preset '%s' (ObjectType: %s) ignores WorldStatic"),
                    *P.PresetName.ToString(), *P.ObjectTypeChannel.ToString()),
                TEXT("Dynamic actors that ignore WorldStatic will fall through the floor"));
            R.AffectedPresets.Add(P.PresetName);
            R.AffectedChannels.Add(GWorldStatic);
            Results.Add(MoveTemp(R));
        }
    }
}


// ---------------------------------------------------------------------------
//  CS_OVERLAP_MISMATCH
//  One preset Blocks the other's ObjectType, but the other Ignores this preset's
//  ObjectType — the Block is silently overridden to Ignore by the min() rule.
//  One result per unordered pair (covers both directions in one result).
// ---------------------------------------------------------------------------
void FCCValidator::CheckOverlapMismatch(const FCCCollisionSnapshot& Snapshot,
                                        TArray<FCCValidationResult>& Results)
{
    const int32 N = Snapshot.Presets.Num();
    for (int32 i = 0; i < N; ++i)
    {
        for (int32 j = i + 1; j < N; ++j)
        {
            const FCCPresetData& A = Snapshot.Presets[i];
            const FCCPresetData& B = Snapshot.Presets[j];

            const ECollisionResponse* AtoB = A.QueryResponses.Find(B.ObjectTypeChannel);
            const ECollisionResponse* BtoA = B.QueryResponses.Find(A.ObjectTypeChannel);
            if (!AtoB || !BtoA) continue;

            const bool bABlocksBIgnores = (*AtoB == ECR_Block) && (*BtoA == ECR_Ignore);
            const bool bBBlocksAIgnores = (*BtoA == ECR_Block) && (*AtoB == ECR_Ignore);

            if (!bABlocksBIgnores && !bBBlocksAIgnores) continue;

            FString Desc;
            if (bABlocksBIgnores && !bBBlocksAIgnores)
            {
                Desc = FString::Printf(
                    TEXT("'%s' wants to Block '%s', but '%s' ignores '%s's ObjectType — result is Ignore"),
                    *A.PresetName.ToString(), *B.PresetName.ToString(),
                    *B.PresetName.ToString(), *A.PresetName.ToString());
            }
            else if (bBBlocksAIgnores && !bABlocksBIgnores)
            {
                Desc = FString::Printf(
                    TEXT("'%s' wants to Block '%s', but '%s' ignores '%s's ObjectType — result is Ignore"),
                    *B.PresetName.ToString(), *A.PresetName.ToString(),
                    *A.PresetName.ToString(), *B.PresetName.ToString());
            }
            else
            {
                // Both directions have a mismatch — each is overriding the other.
                Desc = FString::Printf(
                    TEXT("'%s' and '%s' have asymmetric Block/Ignore responses — both resolve to Ignore"),
                    *A.PresetName.ToString(), *B.PresetName.ToString());
            }

            FCCValidationResult R = MakeResult(
                "CS_OVERLAP_MISMATCH",
                ECCValidationSeverity::Info,
                MoveTemp(Desc),
                TEXT("Review intent — the Block is silently overridden"));
            R.AffectedPresets = { A.PresetName, B.PresetName };
            Results.Add(MoveTemp(R));
        }
    }
}


// ---------------------------------------------------------------------------
//  CS_NAME_CONFLICT
//  Custom preset (bIsBuiltin == false) whose name exactly matches a built-in.
// ---------------------------------------------------------------------------
void FCCValidator::CheckNameConflict(const FCCCollisionSnapshot& Snapshot,
                                     TArray<FCCValidationResult>& Results)
{
    // Build a set of all built-in preset names for O(1) lookup.
    TSet<FName> BuiltinNames;
    for (const FCCPresetData& P : Snapshot.Presets)
    {
        if (P.bIsBuiltin) BuiltinNames.Add(P.PresetName);
    }

    for (const FCCPresetData& P : Snapshot.Presets)
    {
        if (P.bIsBuiltin) continue;
        if (BuiltinNames.Contains(P.PresetName))
        {
            FCCValidationResult R = MakeResult(
                "CS_NAME_CONFLICT",
                ECCValidationSeverity::Error,
                FString::Printf(TEXT("Custom preset '%s' matches a UE built-in preset name exactly"),
                                *P.PresetName.ToString()),
                TEXT("Rename to avoid lookup ambiguity"));
            R.AffectedPresets.Add(P.PresetName);
            Results.Add(MoveTemp(R));
        }
    }
}


// ---------------------------------------------------------------------------
//  CS_OVERLAP_EVENT
//  Pair whose PairwiseMatrix QueryResult == ECR_Overlap.
//  Warns that both actors must have Generate Overlap Events enabled.
//  One result per unordered pair.
// ---------------------------------------------------------------------------
void FCCValidator::CheckOverlapEvent(const FCCCollisionSnapshot& Snapshot,
                                     TArray<FCCValidationResult>& Results)
{
    const int32 N = Snapshot.PresetCount;
    if (N == 0 || Snapshot.PairwiseMatrix.Num() < N * N) return;

    for (int32 i = 0; i < N; ++i)
    {
        for (int32 j = i + 1; j < N; ++j)
        {
            const FCCPairwiseResult& Pair = Snapshot.PairwiseMatrix[i * N + j];
            if (Pair.QueryResult != ECR_Overlap) continue;

            FCCValidationResult R = MakeResult(
                "CS_OVERLAP_EVENT",
                ECCValidationSeverity::Warning,
                FString::Printf(
                    TEXT("'%s' and '%s' resolve to Overlap — confirm Generate Overlap Events is enabled on both actors"),
                    *Snapshot.Presets[i].PresetName.ToString(),
                    *Snapshot.Presets[j].PresetName.ToString()),
                TEXT("Confirm both actors have Generate Overlap Events enabled at runtime"));
            R.AffectedPresets = { Snapshot.Presets[i].PresetName, Snapshot.Presets[j].PresetName };
            Results.Add(MoveTemp(R));
        }
    }
}


// ---------------------------------------------------------------------------
//  RunComponentChecks
//  Entry point for all component-level rules (actor data, not profile data).
// ---------------------------------------------------------------------------
TArray<FCCValidationResult> FCCValidator::RunComponentChecks(
    const TArray<FCCActorComponentData>& ActorData)
{
    TArray<FCCValidationResult> Results;
    CheckComponentCustomPreset(ActorData, Results);
    return Results;
}


// ---------------------------------------------------------------------------
//  CS_COMPONENT_CUSTOM_PRESET
//  Component whose collision profile name is "Custom", meaning the developer
//  manually overrode responses without assigning a named preset.  This bypasses
//  the preset system and makes collision harder to audit.
// ---------------------------------------------------------------------------
void FCCValidator::CheckComponentCustomPreset(const TArray<FCCActorComponentData>& ActorData,
                                              TArray<FCCValidationResult>& Results)
{
    static const FName CustomProfile(TEXT("Custom"));

    for (const FCCActorComponentData& Actor : ActorData)
    {
        for (const FCCComponentInfo& Comp : Actor.Components)
        {
            if (Comp.PresetName != CustomProfile) continue;

            FCCValidationResult R = MakeResult(
                "CS_COMPONENT_CUSTOM_PRESET",
                ECCValidationSeverity::Warning,
                FString::Printf(
                    TEXT("Component '%s' uses the Custom collision profile — "
                         "responses are not managed by a named preset"),
                    *Comp.ComponentName.ToString()),
                TEXT("Create a named preset in Project Settings \u2192 Collision "
                     "and assign it to this component"));
            R.AffectedPresets.Add(CustomProfile);
            Results.Add(MoveTemp(R));
        }
    }
}


// ---------------------------------------------------------------------------
//  CS_VISIBILITY_BLOCK
//  Preset that blocks the Visibility trace channel.
// ---------------------------------------------------------------------------
void FCCValidator::CheckVisibilityBlock(const FCCCollisionSnapshot& Snapshot,
                                        TArray<FCCValidationResult>& Results)
{
    for (const FCCPresetData& P : Snapshot.Presets)
    {
        const ECollisionResponse* Resp = P.QueryResponses.Find(GVisibility);
        if (Resp && *Resp == ECR_Block)
        {
            FCCValidationResult R = MakeResult(
                "CS_VISIBILITY_BLOCK",
                ECCValidationSeverity::Info,
                FString::Printf(TEXT("Preset '%s' blocks the Visibility channel"),
                                *P.PresetName.ToString()),
                TEXT("Confirm intent — affects AI sight, shooting, and camera traces"));
            R.AffectedPresets.Add(P.PresetName);
            R.AffectedChannels.Add(GVisibility);
            Results.Add(MoveTemp(R));
        }
    }
}
