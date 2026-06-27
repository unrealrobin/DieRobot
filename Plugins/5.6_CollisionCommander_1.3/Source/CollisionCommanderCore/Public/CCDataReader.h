// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CCTypes.h"

// ============================================================================
//  FCCDataReader
//
//  Reads all collision data from UCollisionProfile and produces an
//  FCCCollisionSnapshot. This is the single entry point for all data in
//  Collision Commander — nothing else reads from UCollisionProfile directly.
//
//  BuildSnapshot() can be called from tests without any editor or UI context.
// ============================================================================
struct COLLISIONCOMMANDERCORE_API FCCDataReader
{
    // Reads the current project collision state from UCollisionProfile and
    // returns a fully populated snapshot with pairwise results for every
    // preset pair (Query and Physics computed independently).
    static FCCCollisionSnapshot BuildSnapshot();

    // Reads all collision-capable primitive components from an actor and returns
    // per-component collision metadata. Components with NoCollision are excluded.
    // Returns empty FCCActorComponentData if Actor is null.
    static FCCActorComponentData BuildActorComponentData(AActor* Actor);

    // Computes how ExperimentPreset would interact with every preset in Snapshot.
    // Returns a TArray<FCCPairwiseResult> index-aligned with Snapshot.Presets.
    // Uses the same ResolvePair min() rule as the pairwise matrix.
    static TArray<FCCPairwiseResult> ComputeExperimentInteractions(
        const FCCExperimentPreset& Exp,
        const FCCCollisionSnapshot& Snapshot);

    // Returns true if a preset with the given name already exists in UCollisionProfile.
    // Returns false for an empty / NAME_None query or if the profile is unavailable.
    // Only call from editor context — never from Core tests.
    static bool PresetNameExists(FName Name);

    // Writes ExperimentPreset to UCollisionProfile as a new collision profile entry.
    // If bAllowOverwrite is true and a preset with the same name exists, it is
    // overwritten in-place. Returns false (and logs) if PresetName is empty.
    // Only call from editor context — never from Core tests.
    static bool CreatePreset(const FCCExperimentPreset& Exp, bool bAllowOverwrite = false);

    // Resolves the interaction between two presets for one response type.
    // Applies the rule: result = min(A's response to B's ObjectType, B's response to A's ObjectType)
    // ECR_Ignore=0 < ECR_Overlap=1 < ECR_Block=2. If either side is Ignore, result is Ignore.
    //
    // Public so unit tests in CollisionCommanderTests can call it directly.
    // friend declarations don't cross DLL boundaries, so public is the correct
    // visibility for a function that must be linked from a separate test module.
    static ECollisionResponse ResolvePair(const FCCPresetData& A, const FCCPresetData& B,
                                          bool bUseQueryResponses);

private:
    // UE has 14 built-in collision channels (indices 0–13).
    // Channels at index 14+ are project-defined custom channels.
    static constexpr int32 UE_BUILTIN_CHANNEL_COUNT = 14;

    // Reads all named channel slots (0 to ECC_MAX) into the snapshot.
    static void ReadChannels(UCollisionProfile* Profile, FCCCollisionSnapshot& OutSnapshot);

    // Reads all preset definitions into OutPresets and counts built-in presets.
    // Query and Physics responses are populated separately based on CollisionEnabled.
    static void ReadPresets(UCollisionProfile* Profile, const FCCCollisionSnapshot& Snapshot,
                            TArray<FCCPresetData>& OutPresets, int32& OutBuiltinPresetCount);

    // Fills the flat N×N pairwise matrix once channels and presets are ready.
    static void ComputePairwiseMatrix(FCCCollisionSnapshot& OutSnapshot);
};
