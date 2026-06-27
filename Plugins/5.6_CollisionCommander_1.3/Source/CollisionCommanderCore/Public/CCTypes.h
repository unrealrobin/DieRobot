// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

// ============================================================================
//  Collision Commander — Core Data Types
//
//  These structs represent the full collision state of a UE project as read
//  from UCollisionProfile. They are populated by FCCDataReader::BuildSnapshot()
//  and consumed by the UI layer and tests.
//
//  Design rules enforced here:
//    - Query and Physics responses are always stored separately. Never merged.
//    - Channel indices are never hardcoded for custom channels. Names are used.
//    - No Slate, no UI, no editor-only types. This file must stay testable.
// ============================================================================


// A single collision channel (object type or trace channel) as defined in
// Project Settings → Collision. The first 14 are UE built-ins; everything
// after is project-defined.
struct FCCChannelInfo
{
    FName   Name;           // Channel name as registered in the engine
    int32   Index;          // ECollisionChannel cast to int32
    bool    bIsBuiltin;     // True for UE's 14 built-in channels
    bool    bIsTraceChannel = false; // True if no preset uses this channel as its ObjectType
};


// A single collision preset as defined in Project Settings → Collision.
// Holds per-channel responses for both Query and Physics independently.
// ObjectTypeChannel is the channel this preset uses as its own object type.
struct FCCPresetData
{
    FName                           PresetName;
    FName                           ObjectTypeChannel;
    FString                         Description;           // HelpMessage from FCollisionResponseTemplate
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

    // Per-channel responses — keyed by channel name, never by index.
    // Query governs traces and overlaps; Physics governs rigid-body simulation.
    // These must never be collapsed into a single map.
    TMap<FName, ECollisionResponse> QueryResponses;
    TMap<FName, ECollisionResponse> PhysicsResponses;

    bool bIsBuiltin;    // True for UE's built-in presets (NoCollision, Pawn, etc.)
};


// The resolved interaction between two presets.
// Computed as: result = min(A's response to B's ObjectType, B's response to A's ObjectType)
// Computed twice — once for Query, once for Physics. Both are always stored.
struct FCCPairwiseResult
{
    ECollisionResponse QueryResult   = ECR_Ignore;
    ECollisionResponse PhysicsResult = ECR_Ignore;
};


// A single primitive component's collision configuration as read from an AActor.
// Only components with CollisionEnabled != NoCollision are included.
struct FCCComponentInfo
{
    FName                       ComponentName;      // FName unique within the actor
    FString                     ComponentClass;     // UClass display name, e.g. "StaticMeshComponent"
    FName                       PresetName;         // Collision profile name set on this component
    ECollisionEnabled::Type     CollisionEnabled;   // Query/Physics/Both
    FName                       ObjectTypeChannel;  // The channel this component uses as its object type
};


// ============================================================================
//  Validation types
//
//  FCCValidator (Core) produces TArray<FCCValidationResult> from a snapshot.
//  Results are never stored inside FCCCollisionSnapshot — they are a derived
//  view, recomputed on demand by the UI layer.
// ============================================================================

enum class ECCValidationSeverity : uint8
{
    Error,    // Must fix — active misconfiguration or naming collision
    Warning,  // Should review — likely unintended
    Info,     // Worth knowing — may be intentional
};

struct FCCValidationResult
{
    FName                  RuleId;           // e.g. "CS_CHAN_BUDGET_WARN"
    ECCValidationSeverity  Severity;
    FString                Description;      // human-readable trigger description
    FString                SuggestedFix;
    TArray<FName>          AffectedPresets;  // preset names involved (may be empty)
    TArray<FName>          AffectedChannels; // channel names involved (may be empty)
};


// All collision-capable primitive components on a single actor.
struct FCCActorComponentData
{
    TArray<FCCComponentInfo>    Components;  // one entry per UPrimitiveComponent with CollisionEnabled != NoCollision
};


// A complete snapshot of the project's collision state at a point in time.
// Built by FCCDataReader::BuildSnapshot() by reading UCollisionProfile.
//
// PairwiseMatrix is a flat array of size PresetCount * PresetCount.
// Access a cell as: PairwiseMatrix[A * PresetCount + B]
// This gives a single contiguous allocation and cache-friendly iteration
// compared to a jagged TArray<TArray<>>. The full matrix fits in L1 cache
// at typical project sizes (N=50 → ~5 KB).
//
// BuiltinPresetCount marks the boundary between UE built-in presets and
// project-defined custom presets. Presets[0..BuiltinPresetCount-1] are
// built-ins; everything after is custom.
struct FCCCollisionSnapshot
{
    TArray<FCCChannelInfo>      Channels;
    TArray<FCCChannelInfo>      TraceChannels;      // subset of Channels where bIsTraceChannel == true
    TArray<FCCPresetData>       Presets;
    TArray<FCCPairwiseResult>   PairwiseMatrix;     // flat, size = PresetCount^2
    int32                       PresetCount        = 0;
    int32                       BuiltinPresetCount = 0;
};


// An in-memory collision preset being designed in Experiment Mode.
// Not committed to UCollisionProfile — ephemeral for the session only.
// Mirrors FCCPresetData's structure so it can be fed directly to ResolvePair.
struct FCCExperimentPreset
{
    FName   PresetName;
    FName   ObjectTypeChannel;
    FString Description;           // Maps to FCollisionResponseTemplate::HelpMessage
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

    // Per-channel responses — keyed by channel name, never by index.
    // Query and Physics stored separately per spec Hard Rule 4.
    TMap<FName, ECollisionResponse> QueryResponses;
    TMap<FName, ECollisionResponse> PhysicsResponses;

    // Resets to a blank preset: all channels set to ECR_Ignore,
    // ObjectTypeChannel set to the first available channel in the snapshot.
    // PresetName, Description cleared; CollisionEnabled set to QueryAndPhysics.
    void InitFromSnapshot(const FCCCollisionSnapshot& Snapshot)
    {
        PresetName        = NAME_None;
        Description       = FString();
        CollisionEnabled  = ECollisionEnabled::QueryAndPhysics;
        ObjectTypeChannel = Snapshot.Channels.Num() > 0
                            ? Snapshot.Channels[0].Name : NAME_None;
        QueryResponses.Empty();
        PhysicsResponses.Empty();
        for (const FCCChannelInfo& Ch : Snapshot.Channels)
        {
            QueryResponses.Add(Ch.Name,   ECR_Ignore);
            PhysicsResponses.Add(Ch.Name, ECR_Ignore);
        }
    }

    // Copies all values from an existing FCCPresetData (use as a starting point).
    void LoadFromPreset(const FCCPresetData& Source)
    {
        PresetName        = Source.PresetName;
        ObjectTypeChannel = Source.ObjectTypeChannel;
        Description       = Source.Description;
        CollisionEnabled  = Source.CollisionEnabled;
        QueryResponses    = Source.QueryResponses;
        PhysicsResponses  = Source.PhysicsResponses;
    }
};
