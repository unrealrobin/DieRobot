// Copyright 2026 Paracosm. All Rights Reserved.

// ============================================================================
//  CCDataReaderTests.cpp
//
//  Unit tests for FCCDataReader::ResolvePair — the pairwise collision
//  resolution function. Tests are pure computation: they construct minimal
//  FCCPresetData objects directly and call ResolvePair without a live
//  UCollisionProfile. BuildSnapshot() is NOT tested here.
//
//  Each test names two channel FNames ("ChannelA", "ChannelB") and sets up
//  the preset pair so that:
//    - Preset A has ObjectTypeChannel = "ChannelA"
//    - Preset B has ObjectTypeChannel = "ChannelB"
//    - Preset A's response maps contain an entry for "ChannelB" (A's response to B's type)
//    - Preset B's response maps contain an entry for "ChannelA" (B's response to A's type)
//
//  This mirrors exactly how ComputePairwiseMatrix feeds ResolvePair.
// ============================================================================

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "CCTypes.h"
#include "CCDataReader.h"
#include "FCCValidator.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"

// EAutomationTestFlags_ApplicationContextMask was introduced in UE 5.5.
// Define it for 5.3 / 5.4 so the test macros compile on all supported engines.
#if ENGINE_MINOR_VERSION < 5
#define EAutomationTestFlags_ApplicationContextMask EAutomationTestFlags::ApplicationContextMask
#endif


// ----------------------------------------------------------------------------
//  Helper — builds a minimal FCCPresetData with the given object type channel
//  and a single response map entry for the peer's object type channel.
//  Both QueryResponses and PhysicsResponses are populated independently.
// ----------------------------------------------------------------------------
namespace
{
    FCCPresetData MakePreset(
        FName PresetName,
        FName OwnChannel,
        FName PeerChannel,
        ECollisionResponse QueryResponseToPeer,
        ECollisionResponse PhysicsResponseToPeer)
    {
        FCCPresetData P;
        P.PresetName       = PresetName;
        P.ObjectTypeChannel = OwnChannel;
        P.bIsBuiltin       = false;
        P.QueryResponses.Add(PeerChannel, QueryResponseToPeer);
        P.PhysicsResponses.Add(PeerChannel, PhysicsResponseToPeer);
        return P;
    }
} // anonymous namespace


// ============================================================================
//  Test 1 — Both Block → Block
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCResolvePair_BothBlock,
    "CollisionCommander.Core.PairwiseResolution.BothBlock",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCResolvePair_BothBlock::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelB("ChannelB");

    FCCPresetData A = MakePreset("PresetA", ChannelA, ChannelB, ECR_Block, ECR_Block);
    FCCPresetData B = MakePreset("PresetB", ChannelB, ChannelA, ECR_Block, ECR_Block);

    TestEqual("Query:   Block + Block = Block",   FCCDataReader::ResolvePair(A, B, true),  ECR_Block);
    TestEqual("Physics: Block + Block = Block",   FCCDataReader::ResolvePair(A, B, false), ECR_Block);
    return true;
}


// ============================================================================
//  Test 2 — Both Overlap → Overlap
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCResolvePair_BothOverlap,
    "CollisionCommander.Core.PairwiseResolution.BothOverlap",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCResolvePair_BothOverlap::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelB("ChannelB");

    FCCPresetData A = MakePreset("PresetA", ChannelA, ChannelB, ECR_Overlap, ECR_Overlap);
    FCCPresetData B = MakePreset("PresetB", ChannelB, ChannelA, ECR_Overlap, ECR_Overlap);

    TestEqual("Query:   Overlap + Overlap = Overlap", FCCDataReader::ResolvePair(A, B, true),  ECR_Overlap);
    TestEqual("Physics: Overlap + Overlap = Overlap", FCCDataReader::ResolvePair(A, B, false), ECR_Overlap);
    return true;
}


// ============================================================================
//  Test 3 — Both Ignore → Ignore
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCResolvePair_BothIgnore,
    "CollisionCommander.Core.PairwiseResolution.BothIgnore",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCResolvePair_BothIgnore::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelB("ChannelB");

    FCCPresetData A = MakePreset("PresetA", ChannelA, ChannelB, ECR_Ignore, ECR_Ignore);
    FCCPresetData B = MakePreset("PresetB", ChannelB, ChannelA, ECR_Ignore, ECR_Ignore);

    TestEqual("Query:   Ignore + Ignore = Ignore", FCCDataReader::ResolvePair(A, B, true),  ECR_Ignore);
    TestEqual("Physics: Ignore + Ignore = Ignore", FCCDataReader::ResolvePair(A, B, false), ECR_Ignore);
    return true;
}


// ============================================================================
//  Test 4 — Asymmetric: Block vs Ignore → Ignore
//
//  A responds Block to B's object type; B responds Ignore to A's object type.
//  Either side being Ignore forces the result to Ignore.
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCResolvePair_AsymmetricBlockIgnore,
    "CollisionCommander.Core.PairwiseResolution.AsymmetricBlockIgnore",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCResolvePair_AsymmetricBlockIgnore::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelB("ChannelB");

    // A→B = Block, B→A = Ignore  →  min(2,0) = 0 = Ignore
    FCCPresetData A = MakePreset("PresetA", ChannelA, ChannelB, ECR_Block, ECR_Block);
    FCCPresetData B = MakePreset("PresetB", ChannelB, ChannelA, ECR_Ignore, ECR_Ignore);

    TestEqual("Query:   Block + Ignore = Ignore", FCCDataReader::ResolvePair(A, B, true),  ECR_Ignore);
    TestEqual("Physics: Block + Ignore = Ignore", FCCDataReader::ResolvePair(A, B, false), ECR_Ignore);

    // Also verify symmetry — ResolvePair(B, A) should yield the same result.
    TestEqual("Query:   Ignore + Block = Ignore (reversed)", FCCDataReader::ResolvePair(B, A, true),  ECR_Ignore);
    TestEqual("Physics: Ignore + Block = Ignore (reversed)", FCCDataReader::ResolvePair(B, A, false), ECR_Ignore);
    return true;
}


// ============================================================================
//  Test 5 — Asymmetric: Overlap vs Ignore → Ignore
//
//  Complements Test 4: confirms Ignore dominates Overlap as well as Block.
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCResolvePair_AsymmetricOverlapIgnore,
    "CollisionCommander.Core.PairwiseResolution.AsymmetricOverlapIgnore",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCResolvePair_AsymmetricOverlapIgnore::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelB("ChannelB");

    // A→B = Overlap, B→A = Ignore  →  min(1,0) = 0 = Ignore
    FCCPresetData A = MakePreset("PresetA", ChannelA, ChannelB, ECR_Overlap, ECR_Overlap);
    FCCPresetData B = MakePreset("PresetB", ChannelB, ChannelA, ECR_Ignore, ECR_Ignore);

    TestEqual("Query:   Overlap + Ignore = Ignore", FCCDataReader::ResolvePair(A, B, true),  ECR_Ignore);
    TestEqual("Physics: Overlap + Ignore = Ignore", FCCDataReader::ResolvePair(A, B, false), ECR_Ignore);
    return true;
}


// ============================================================================
//  Test 6 — Mixed: Block vs Overlap → Overlap
//
//  min(Block=2, Overlap=1) = 1 = Overlap. The more permissive side wins.
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCResolvePair_MixedBlockOverlap,
    "CollisionCommander.Core.PairwiseResolution.MixedBlockOverlap",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCResolvePair_MixedBlockOverlap::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelB("ChannelB");

    // A→B = Block, B→A = Overlap  →  min(2,1) = 1 = Overlap
    FCCPresetData A = MakePreset("PresetA", ChannelA, ChannelB, ECR_Block, ECR_Block);
    FCCPresetData B = MakePreset("PresetB", ChannelB, ChannelA, ECR_Overlap, ECR_Overlap);

    TestEqual("Query:   Block + Overlap = Overlap", FCCDataReader::ResolvePair(A, B, true),  ECR_Overlap);
    TestEqual("Physics: Block + Overlap = Overlap", FCCDataReader::ResolvePair(A, B, false), ECR_Overlap);
    return true;
}


// ============================================================================
//  Test 7 — All-Ignore preset: any pairing resolves to Ignore
//
//  A preset with all-Ignore responses must produce Ignore when paired with
//  any other preset regardless of what the other preset responds.
//  Tests Block, Overlap, and Ignore as the peer response.
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCResolvePair_AllIgnorePreset,
    "CollisionCommander.Core.PairwiseResolution.AllIgnorePreset",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCResolvePair_AllIgnorePreset::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelB("ChannelB");

    // AllIgnore preset responds Ignore to everything
    FCCPresetData AllIgnore = MakePreset("AllIgnore", ChannelA, ChannelB, ECR_Ignore, ECR_Ignore);

    // Peer with Block response
    {
        FCCPresetData Blocker = MakePreset("Blocker", ChannelB, ChannelA, ECR_Block, ECR_Block);
        TestEqual("Query:   AllIgnore vs Block = Ignore",   FCCDataReader::ResolvePair(AllIgnore, Blocker, true),  ECR_Ignore);
        TestEqual("Physics: AllIgnore vs Block = Ignore",   FCCDataReader::ResolvePair(AllIgnore, Blocker, false), ECR_Ignore);
        TestEqual("Query:   Block vs AllIgnore = Ignore",   FCCDataReader::ResolvePair(Blocker, AllIgnore, true),  ECR_Ignore);
        TestEqual("Physics: Block vs AllIgnore = Ignore",   FCCDataReader::ResolvePair(Blocker, AllIgnore, false), ECR_Ignore);
    }

    // Peer with Overlap response
    {
        FCCPresetData Overlapper = MakePreset("Overlapper", ChannelB, ChannelA, ECR_Overlap, ECR_Overlap);
        TestEqual("Query:   AllIgnore vs Overlap = Ignore", FCCDataReader::ResolvePair(AllIgnore, Overlapper, true),  ECR_Ignore);
        TestEqual("Physics: AllIgnore vs Overlap = Ignore", FCCDataReader::ResolvePair(AllIgnore, Overlapper, false), ECR_Ignore);
    }

    // Peer also Ignore
    {
        FCCPresetData AlsoIgnore = MakePreset("AlsoIgnore", ChannelB, ChannelA, ECR_Ignore, ECR_Ignore);
        TestEqual("Query:   AllIgnore vs AllIgnore = Ignore", FCCDataReader::ResolvePair(AllIgnore, AlsoIgnore, true),  ECR_Ignore);
        TestEqual("Physics: AllIgnore vs AllIgnore = Ignore", FCCDataReader::ResolvePair(AllIgnore, AlsoIgnore, false), ECR_Ignore);
    }

    return true;
}


// ============================================================================
//  Test 8 — Query != Physics (CRITICAL)
//
//  This is the most important test. It verifies that Query and Physics
//  responses are stored and evaluated independently — never merged.
//
//  Setup:
//    A: QueryResponses[ChannelB] = Block,   PhysicsResponses[ChannelB] = Ignore
//    B: QueryResponses[ChannelA] = Block,   PhysicsResponses[ChannelA] = Block
//
//  Expected:
//    Query result   = min(A→B query=Block,   B→A query=Block)   = Block
//    Physics result = min(A→B physics=Ignore, B→A physics=Block) = Ignore
//
//  If Query and Physics were ever merged or one was used in place of the other,
//  at least one of these assertions would fail.
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCResolvePair_QueryNotEqualPhysics,
    "CollisionCommander.Core.PairwiseResolution.QueryNotEqualPhysics",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCResolvePair_QueryNotEqualPhysics::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelB("ChannelB");

    //  A: Block for Query, Ignore for Physics (to B's channel)
    FCCPresetData A = MakePreset("PresetA", ChannelA, ChannelB,
        /*QueryResponseToPeer=*/   ECR_Block,
        /*PhysicsResponseToPeer=*/ ECR_Ignore);

    //  B: Block for both Query and Physics (to A's channel)
    FCCPresetData B = MakePreset("PresetB", ChannelB, ChannelA,
        /*QueryResponseToPeer=*/   ECR_Block,
        /*PhysicsResponseToPeer=*/ ECR_Block);

    // Query: min(Block, Block) = Block
    TestEqual("Query result must be Block",
        FCCDataReader::ResolvePair(A, B, true),
        ECR_Block);

    // Physics: min(Ignore, Block) = Ignore
    TestEqual("Physics result must be Ignore (Query and Physics are independent)",
        FCCDataReader::ResolvePair(A, B, false),
        ECR_Ignore);

    // Guard: the two results must differ — if they are equal, the independence
    // invariant has been violated even if both individual assertions above passed.
    const ECollisionResponse QueryResult   = FCCDataReader::ResolvePair(A, B, true);
    const ECollisionResponse PhysicsResult = FCCDataReader::ResolvePair(A, B, false);
    TestNotEqual("Query and Physics results must differ for this preset pair",
        (int32)QueryResult,
        (int32)PhysicsResult);

    return true;
}


// ============================================================================
//  Test 9 — Missing channel entry → Ignore (safe default)
//
//  If a preset's response map does not contain an entry for the peer's object
//  type channel, ResolvePair must return Ignore rather than crash or produce
//  a garbage result. This guards the FindChecked / Find path in the implementation.
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCResolvePair_MissingChannelEntry,
    "CollisionCommander.Core.PairwiseResolution.MissingChannelEntry",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCResolvePair_MissingChannelEntry::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelB("ChannelB");
    const FName ChannelC("ChannelC");  // not in any response map

    // B uses ChannelC as its object type, which A has no entry for.
    FCCPresetData A = MakePreset("PresetA", ChannelA, ChannelB, ECR_Block, ECR_Block);
    FCCPresetData B = MakePreset("PresetB", ChannelC, ChannelA, ECR_Block, ECR_Block);
    // A.QueryResponses has "ChannelB" but not "ChannelC" — Find(B.ObjectTypeChannel) returns null.

    TestEqual("Query:   missing channel entry = Ignore",   FCCDataReader::ResolvePair(A, B, true),  ECR_Ignore);
    TestEqual("Physics: missing channel entry = Ignore",   FCCDataReader::ResolvePair(A, B, false), ECR_Ignore);
    return true;
}


// ============================================================================
//  FCCExperimentPreset + ComputeExperimentInteractions tests
//
//  These tests are pure computation — no UCollisionProfile needed.
//  Helper: MakeSnapshot builds a minimal FCCCollisionSnapshot from a list of
//  (preset, channel) pairs using the existing MakePreset helper.
// ============================================================================

namespace
{
    // Builds a minimal FCCCollisionSnapshot containing two presets that respond
    // to each other. Channels: ChannelA (index 0) and ChannelB (index 1).
    // Used by the experiment interaction tests.
    FCCCollisionSnapshot MakeTwoPresetSnapshot()
    {
        const FName ChannelA("ChannelA");
        const FName ChannelB("ChannelB");

        FCCChannelInfo ChA;
        ChA.Name = ChannelA; ChA.Index = 0; ChA.bIsBuiltin = false;

        FCCChannelInfo ChB;
        ChB.Name = ChannelB; ChB.Index = 1; ChB.bIsBuiltin = false;

        FCCCollisionSnapshot Snap;
        Snap.Channels = { ChA, ChB };
        Snap.Presets.Add(MakePreset("PresetA", ChannelA, ChannelB, ECR_Block,   ECR_Block));
        Snap.Presets.Add(MakePreset("PresetB", ChannelB, ChannelA, ECR_Overlap, ECR_Overlap));
        Snap.PresetCount        = Snap.Presets.Num();
        Snap.BuiltinPresetCount = 0;
        return Snap;
    }
} // anonymous namespace


// ============================================================================
//  Test 16 — InitFromSnapshot: all Ignore, ObjectTypeChannel set to first channel
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCExperiment_InitFromSnapshot,
    "CollisionCommander.Core.ExperimentPreset.InitFromSnapshot",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCExperiment_InitFromSnapshot::RunTest(const FString& Parameters)
{
    FCCCollisionSnapshot Snap = MakeTwoPresetSnapshot();

    FCCExperimentPreset Exp;
    Exp.InitFromSnapshot(Snap);

    TestTrue("PresetName cleared", Exp.PresetName.IsNone());
    TestEqual("ObjectTypeChannel = first channel", Exp.ObjectTypeChannel, FName("ChannelA"));
    TestEqual("QueryResponses count = channel count",  Exp.QueryResponses.Num(),   2);
    TestEqual("PhysicsResponses count = channel count", Exp.PhysicsResponses.Num(), 2);

    for (const FCCChannelInfo& Ch : Snap.Channels)
    {
        const ECollisionResponse* Q = Exp.QueryResponses.Find(Ch.Name);
        const ECollisionResponse* P = Exp.PhysicsResponses.Find(Ch.Name);
        TestNotNull(FString::Printf(TEXT("QueryResponses has entry for %s"), *Ch.Name.ToString()),   Q);
        TestNotNull(FString::Printf(TEXT("PhysicsResponses has entry for %s"), *Ch.Name.ToString()), P);
        if (Q) { TestEqual("QueryResponse is Ignore",   *Q, ECR_Ignore); }
        if (P) { TestEqual("PhysicsResponse is Ignore", *P, ECR_Ignore); }
    }
    return true;
}


// ============================================================================
//  Test 17 — LoadFromPreset: copies name, ObjType, and all response maps exactly
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCExperiment_LoadFromPreset,
    "CollisionCommander.Core.ExperimentPreset.LoadFromPreset",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCExperiment_LoadFromPreset::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelB("ChannelB");
    FCCPresetData Source = MakePreset("SourcePreset", ChannelA, ChannelB, ECR_Block, ECR_Overlap);

    FCCExperimentPreset Exp;
    Exp.LoadFromPreset(Source);

    TestEqual("PresetName copied",          Exp.PresetName,        FName("SourcePreset"));
    TestEqual("ObjectTypeChannel copied",   Exp.ObjectTypeChannel, ChannelA);
    TestEqual("QueryResponses count",       Exp.QueryResponses.Num(),   Source.QueryResponses.Num());
    TestEqual("PhysicsResponses count",     Exp.PhysicsResponses.Num(), Source.PhysicsResponses.Num());

    const ECollisionResponse* Q = Exp.QueryResponses.Find(ChannelB);
    const ECollisionResponse* P = Exp.PhysicsResponses.Find(ChannelB);
    TestNotNull("QueryResponses has ChannelB",   Q);
    TestNotNull("PhysicsResponses has ChannelB", P);
    if (Q) { TestEqual("QueryResponse = Block",   *Q, ECR_Block);   }
    if (P) { TestEqual("PhysicsResponse = Overlap", *P, ECR_Overlap); }

    return true;
}


// ============================================================================
//  Test 18 — ComputeExperimentInteractions: empty snapshot → empty result
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCExperiment_EmptySnapshot,
    "CollisionCommander.Core.ExperimentInteractions.EmptySnapshot",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCExperiment_EmptySnapshot::RunTest(const FString& Parameters)
{
    FCCCollisionSnapshot EmptySnap;
    EmptySnap.PresetCount = 0;

    FCCExperimentPreset Exp;
    TArray<FCCPairwiseResult> Results = FCCDataReader::ComputeExperimentInteractions(Exp, EmptySnap);

    TestEqual("Empty snapshot → empty results", Results.Num(), 0);
    return true;
}


// ============================================================================
//  Test 19 — All-Ignore experiment vs blocking preset → all results Ignore
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCExperiment_AllIgnoreVsBlock,
    "CollisionCommander.Core.ExperimentInteractions.AllIgnoreVsBlock",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCExperiment_AllIgnoreVsBlock::RunTest(const FString& Parameters)
{
    FCCCollisionSnapshot Snap = MakeTwoPresetSnapshot();

    // Experiment preset: all Ignore, ObjType = ChannelX (unrelated)
    FCCExperimentPreset Exp;
    Exp.PresetName        = FName("TestExp");
    Exp.ObjectTypeChannel = FName("ChannelX");
    Exp.QueryResponses.Add(FName("ChannelA"), ECR_Ignore);
    Exp.QueryResponses.Add(FName("ChannelB"), ECR_Ignore);
    Exp.PhysicsResponses.Add(FName("ChannelA"), ECR_Ignore);
    Exp.PhysicsResponses.Add(FName("ChannelB"), ECR_Ignore);

    TArray<FCCPairwiseResult> Results = FCCDataReader::ComputeExperimentInteractions(Exp, Snap);

    TestEqual("Result count equals preset count", Results.Num(), Snap.Presets.Num());
    for (int32 i = 0; i < Results.Num(); ++i)
    {
        TestEqual(FString::Printf(TEXT("Query result[%d] = Ignore"),   i), Results[i].QueryResult,   ECR_Ignore);
        TestEqual(FString::Printf(TEXT("Physics result[%d] = Ignore"), i), Results[i].PhysicsResult, ECR_Ignore);
    }
    return true;
}


// ============================================================================
//  Test 20 — Experiment blocks ChannelA; PresetA has ObjType ChannelA
//            and blocks back → result is Block
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCExperiment_MutualBlock,
    "CollisionCommander.Core.ExperimentInteractions.MutualBlock",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCExperiment_MutualBlock::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelB("ChannelB");
    const FName ChannelExp("ChannelExp");

    // Snapshot: one preset whose ObjType = ChannelExp and it blocks ChannelExp.
    FCCChannelInfo ChA;  ChA.Name = ChannelA;   ChA.Index = 0; ChA.bIsBuiltin = false;
    FCCChannelInfo ChExp; ChExp.Name = ChannelExp; ChExp.Index = 1; ChExp.bIsBuiltin = false;

    FCCPresetData ExistingPreset;
    ExistingPreset.PresetName        = FName("Existing");
    ExistingPreset.ObjectTypeChannel = ChannelA;
    ExistingPreset.bIsBuiltin        = false;
    ExistingPreset.QueryResponses.Add(ChannelExp, ECR_Block);
    ExistingPreset.PhysicsResponses.Add(ChannelExp, ECR_Block);

    FCCCollisionSnapshot Snap;
    Snap.Channels = { ChA, ChExp };
    Snap.Presets  = { ExistingPreset };
    Snap.PresetCount        = 1;
    Snap.BuiltinPresetCount = 0;

    // Experiment: ObjType = ChannelExp, blocks ChannelA (ExistingPreset's ObjType).
    FCCExperimentPreset Exp;
    Exp.PresetName        = FName("MyExp");
    Exp.ObjectTypeChannel = ChannelExp;
    Exp.QueryResponses.Add(ChannelA,   ECR_Block);
    Exp.PhysicsResponses.Add(ChannelA, ECR_Block);

    // Expected: min(Exp→Existing's ObjType=Block, Existing→Exp's ObjType=Block) = Block
    TArray<FCCPairwiseResult> Results = FCCDataReader::ComputeExperimentInteractions(Exp, Snap);
    TestEqual("Result count", Results.Num(), 1);
    if (Results.Num() == 1)
    {
        TestEqual("Query result = Block",   Results[0].QueryResult,   ECR_Block);
        TestEqual("Physics result = Block", Results[0].PhysicsResult, ECR_Block);
    }
    return true;
}


// ============================================================================
//  Test 21 — Query and Physics computed independently on experiment interactions
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCExperiment_QueryPhysicsIndependent,
    "CollisionCommander.Core.ExperimentInteractions.QueryPhysicsIndependent",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCExperiment_QueryPhysicsIndependent::RunTest(const FString& Parameters)
{
    const FName ChannelA("ChannelA");
    const FName ChannelExp("ChannelExp");

    FCCChannelInfo ChA;   ChA.Name = ChannelA;   ChA.Index = 0; ChA.bIsBuiltin = false;
    FCCChannelInfo ChExp; ChExp.Name = ChannelExp; ChExp.Index = 1; ChExp.bIsBuiltin = false;

    // Existing preset blocks ChannelExp for both Query and Physics.
    FCCPresetData Existing;
    Existing.PresetName        = FName("Existing");
    Existing.ObjectTypeChannel = ChannelA;
    Existing.bIsBuiltin        = false;
    Existing.QueryResponses.Add(ChannelExp,   ECR_Block);
    Existing.PhysicsResponses.Add(ChannelExp, ECR_Block);

    FCCCollisionSnapshot Snap;
    Snap.Channels = { ChA, ChExp };
    Snap.Presets  = { Existing };
    Snap.PresetCount = 1;

    // Experiment: blocks ChannelA for Query, Ignores ChannelA for Physics.
    FCCExperimentPreset Exp;
    Exp.PresetName        = FName("Exp");
    Exp.ObjectTypeChannel = ChannelExp;
    Exp.QueryResponses.Add(ChannelA,   ECR_Block);
    Exp.PhysicsResponses.Add(ChannelA, ECR_Ignore);

    // Query: min(Exp→ChannelA=Block, Existing→ChannelExp=Block) = Block
    // Physics: min(Exp→ChannelA=Ignore, Existing→ChannelExp=Block) = Ignore
    TArray<FCCPairwiseResult> Results = FCCDataReader::ComputeExperimentInteractions(Exp, Snap);
    TestEqual("Result count", Results.Num(), 1);
    if (Results.Num() == 1)
    {
        TestEqual("Query = Block",   Results[0].QueryResult,   ECR_Block);
        TestEqual("Physics = Ignore", Results[0].PhysicsResult, ECR_Ignore);
        TestNotEqual("Query and Physics differ",
            (int32)Results[0].QueryResult, (int32)Results[0].PhysicsResult);
    }
    return true;
}


// ============================================================================
//  BuildActorComponentData tests
//
//  Each test constructs actors and components directly via NewObject without
//  spinning up a world or PIE session. Components are added to the actor's
//  OwnedComponents via AddOwnedComponent so GetComponents<UPrimitiveComponent>
//  finds them, matching exactly what live actors provide.
// ============================================================================

namespace
{
    // Creates a UStaticMeshComponent, sets its collision enabled state, and
    // registers it with the actor's owned component list.
    UStaticMeshComponent* MakeAndAddComponent(
        AActor* Actor,
        FName   CompName,
        ECollisionEnabled::Type Enabled)
    {
        UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(Actor, CompName);
        Comp->SetCollisionEnabled(Enabled);
        Actor->AddOwnedComponent(Comp);
        return Comp;
    }
} // anonymous namespace


// ============================================================================
//  Test 10 — Null actor → empty result, no crash
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCBuildActorData_NullActor,
    "CollisionCommander.Core.ActorComponentData.NullActor",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCBuildActorData_NullActor::RunTest(const FString& Parameters)
{
    FCCActorComponentData Data = FCCDataReader::BuildActorComponentData(nullptr);
    TestEqual("Null actor → empty Components array", Data.Components.Num(), 0);
    return true;
}


// ============================================================================
//  Test 11 — Actor with no PrimitiveComponents → empty
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCBuildActorData_NoComponents,
    "CollisionCommander.Core.ActorComponentData.NoComponents",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCBuildActorData_NoComponents::RunTest(const FString& Parameters)
{
    AActor* Actor = NewObject<AActor>(GetTransientPackage());
    FCCActorComponentData Data = FCCDataReader::BuildActorComponentData(Actor);
    TestEqual("Actor with no components → empty", Data.Components.Num(), 0);
    return true;
}


// ============================================================================
//  Test 12 — NoCollision component is filtered out
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCBuildActorData_SkipsNoCollision,
    "CollisionCommander.Core.ActorComponentData.SkipsNoCollision",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCBuildActorData_SkipsNoCollision::RunTest(const FString& Parameters)
{
    AActor* Actor = NewObject<AActor>(GetTransientPackage());
    MakeAndAddComponent(Actor, FName("NoCollComp"), ECollisionEnabled::NoCollision);

    FCCActorComponentData Data = FCCDataReader::BuildActorComponentData(Actor);
    TestEqual("NoCollision component skipped → empty", Data.Components.Num(), 0);
    return true;
}


// ============================================================================
//  Test 13 — QueryOnly component is included with correct metadata
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCBuildActorData_IncludesQueryOnly,
    "CollisionCommander.Core.ActorComponentData.IncludesQueryOnly",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCBuildActorData_IncludesQueryOnly::RunTest(const FString& Parameters)
{
    const FName CompName("QueryComp");
    AActor* Actor = NewObject<AActor>(GetTransientPackage());
    MakeAndAddComponent(Actor, CompName, ECollisionEnabled::QueryOnly);

    FCCActorComponentData Data = FCCDataReader::BuildActorComponentData(Actor);
    TestEqual("QueryOnly component included", Data.Components.Num(), 1);

    if (Data.Components.Num() == 1)
    {
        TestEqual("ComponentName matches",     Data.Components[0].ComponentName,    CompName);
        TestEqual("ComponentClass is correct", Data.Components[0].ComponentClass,   FString(TEXT("StaticMeshComponent")));
        TestEqual("CollisionEnabled is QueryOnly",
            Data.Components[0].CollisionEnabled, ECollisionEnabled::QueryOnly);
    }
    return true;
}


// ============================================================================
//  Test 14 — QueryAndPhysics component is included with correct metadata
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCBuildActorData_IncludesQueryAndPhysics,
    "CollisionCommander.Core.ActorComponentData.IncludesQueryAndPhysics",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCBuildActorData_IncludesQueryAndPhysics::RunTest(const FString& Parameters)
{
    const FName CompName("PhysicsComp");
    AActor* Actor = NewObject<AActor>(GetTransientPackage());
    MakeAndAddComponent(Actor, CompName, ECollisionEnabled::QueryAndPhysics);

    FCCActorComponentData Data = FCCDataReader::BuildActorComponentData(Actor);
    TestEqual("QueryAndPhysics component included", Data.Components.Num(), 1);

    if (Data.Components.Num() == 1)
    {
        TestEqual("ComponentName matches",     Data.Components[0].ComponentName,    CompName);
        TestEqual("CollisionEnabled is QueryAndPhysics",
            Data.Components[0].CollisionEnabled, ECollisionEnabled::QueryAndPhysics);
    }
    return true;
}


// ============================================================================
//  Test 15 — Two components: one NoCollision (filtered) + one QueryOnly (kept)
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCBuildActorData_FiltersMixedComponents,
    "CollisionCommander.Core.ActorComponentData.FiltersMixedComponents",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCBuildActorData_FiltersMixedComponents::RunTest(const FString& Parameters)
{
    const FName FilteredName("NoCollComp");
    const FName KeptName("QueryComp");

    AActor* Actor = NewObject<AActor>(GetTransientPackage());
    MakeAndAddComponent(Actor, FilteredName, ECollisionEnabled::NoCollision);
    MakeAndAddComponent(Actor, KeptName,     ECollisionEnabled::QueryOnly);

    FCCActorComponentData Data = FCCDataReader::BuildActorComponentData(Actor);
    TestEqual("Only QueryOnly component kept", Data.Components.Num(), 1);

    if (Data.Components.Num() == 1)
    {
        TestEqual("Kept component is the QueryOnly one",
            Data.Components[0].ComponentName, KeptName);
        TestEqual("Kept component has QueryOnly state",
            Data.Components[0].CollisionEnabled, ECollisionEnabled::QueryOnly);
    }
    return true;
}


// ============================================================================
//  FCCValidator tests (Tests 22–32)
//
//  Each test builds a minimal FCCCollisionSnapshot that triggers exactly one
//  validation rule.  For each test we assert:
//    - The expected rule ID is present in RunAll() output.
//    - The expected severity is correct.
//    - No other rules fire (snapshot is otherwise clean).
//
//  All tests are pure struct construction — no live UCollisionProfile needed.
// ============================================================================

namespace
{
    FCCChannelInfo MakeCustomChannel(FName Name, int32 Index)
    {
        FCCChannelInfo Ch;
        Ch.Name      = Name;
        Ch.Index     = Index;
        Ch.bIsBuiltin = false;
        return Ch;
    }

    FCCChannelInfo MakeBuiltinChannel(FName Name, int32 Index)
    {
        FCCChannelInfo Ch;
        Ch.Name      = Name;
        Ch.Index     = Index;
        Ch.bIsBuiltin = true;
        return Ch;
    }

    bool HasRule(const TArray<FCCValidationResult>& Results, FName RuleId)
    {
        for (const FCCValidationResult& R : Results)
        {
            if (R.RuleId == RuleId) return true;
        }
        return false;
    }

    const FCCValidationResult* FindRule(const TArray<FCCValidationResult>& Results, FName RuleId)
    {
        for (const FCCValidationResult& R : Results)
        {
            if (R.RuleId == RuleId) return &R;
        }
        return nullptr;
    }
} // anonymous namespace


// ============================================================================
//  Test 22 — CS_CHAN_BUDGET_WARN: exactly 14 custom channels → Warning
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCValidator_ChanBudgetWarn,
    "CollisionCommander.Core.Validator.ChanBudgetWarn",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCValidator_ChanBudgetWarn::RunTest(const FString& Parameters)
{
    // 14 custom channels triggers WARN (>=14) but not CRIT (<17).
    // One preset blocks every channel to suppress CS_CHAN_UNUSED.
    FCCCollisionSnapshot Snap;
    for (int32 i = 0; i < 14; ++i)
    {
        Snap.Channels.Add(MakeCustomChannel(
            FName(*FString::Printf(TEXT("CustomChan%d"), i)), i));
    }

    FCCPresetData P;
    P.PresetName        = FName("TestPreset");
    P.ObjectTypeChannel = Snap.Channels[0].Name;
    P.bIsBuiltin        = false;
    for (const FCCChannelInfo& Ch : Snap.Channels)
    {
        P.QueryResponses.Add(Ch.Name, ECR_Block);
    }
    Snap.Presets.Add(P);
    Snap.PresetCount = Snap.Presets.Num();

    const TArray<FCCValidationResult> Results = FCCValidator::RunAll(Snap);

    TestEqual("Exactly one result",                          Results.Num(), 1);
    TestTrue("CS_CHAN_BUDGET_WARN fires",                     HasRule(Results, "CS_CHAN_BUDGET_WARN"));
    TestFalse("CS_CHAN_BUDGET_CRIT does not fire",            HasRule(Results, "CS_CHAN_BUDGET_CRIT"));

    const FCCValidationResult* R = FindRule(Results, "CS_CHAN_BUDGET_WARN");
    if (R)
    {
        TestEqual("Severity = Warning", R->Severity, ECCValidationSeverity::Warning);
    }
    return true;
}


// ============================================================================
//  Test 23 — CS_CHAN_BUDGET_CRIT: 17 custom channels → Error; WARN suppressed
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCValidator_ChanBudgetCrit,
    "CollisionCommander.Core.Validator.ChanBudgetCrit",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCValidator_ChanBudgetCrit::RunTest(const FString& Parameters)
{
    // 17 custom channels triggers CRIT (>=17).
    // The if/else in CheckChanBudget means WARN's branch is skipped entirely.
    FCCCollisionSnapshot Snap;
    for (int32 i = 0; i < 17; ++i)
    {
        Snap.Channels.Add(MakeCustomChannel(
            FName(*FString::Printf(TEXT("CustomChan%d"), i)), i));
    }

    FCCPresetData P;
    P.PresetName        = FName("TestPreset");
    P.ObjectTypeChannel = Snap.Channels[0].Name;
    P.bIsBuiltin        = false;
    for (const FCCChannelInfo& Ch : Snap.Channels)
    {
        P.QueryResponses.Add(Ch.Name, ECR_Block);
    }
    Snap.Presets.Add(P);
    Snap.PresetCount = Snap.Presets.Num();

    const TArray<FCCValidationResult> Results = FCCValidator::RunAll(Snap);

    TestEqual("Exactly one result",                                    Results.Num(), 1);
    TestTrue("CS_CHAN_BUDGET_CRIT fires",                               HasRule(Results, "CS_CHAN_BUDGET_CRIT"));
    TestFalse("CS_CHAN_BUDGET_WARN does not fire (CRIT takes priority)", HasRule(Results, "CS_CHAN_BUDGET_WARN"));

    const FCCValidationResult* R = FindRule(Results, "CS_CHAN_BUDGET_CRIT");
    if (R)
    {
        TestEqual("Severity = Error", R->Severity, ECCValidationSeverity::Error);
    }
    return true;
}


// ============================================================================
//  Test 24 — CS_CHAN_UNUSED: one custom channel, no presets → Info
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCValidator_ChanUnused,
    "CollisionCommander.Core.Validator.ChanUnused",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCValidator_ChanUnused::RunTest(const FString& Parameters)
{
    // One custom channel with no preset responding to it — exactly CS_CHAN_UNUSED.
    FCCCollisionSnapshot Snap;
    Snap.Channels.Add(MakeCustomChannel(FName("UnusedChan"), 0));
    Snap.PresetCount = 0;
    // No presets → no budget/duplicate/all-ignore/mismatch/overlap rules fire.

    const TArray<FCCValidationResult> Results = FCCValidator::RunAll(Snap);

    TestEqual("Exactly one result",      Results.Num(), 1);
    TestTrue("CS_CHAN_UNUSED fires",      HasRule(Results, "CS_CHAN_UNUSED"));

    const FCCValidationResult* R = FindRule(Results, "CS_CHAN_UNUSED");
    if (R)
    {
        TestEqual("Severity = Info",                     R->Severity, ECCValidationSeverity::Info);
        TestEqual("AffectedChannels contains one entry", R->AffectedChannels.Num(), 1);
    }
    return true;
}


// ============================================================================
//  Test 25 — CS_PRESET_DUPLICATE: two presets share identical ObjType+QueryResponses
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCValidator_PresetDuplicate,
    "CollisionCommander.Core.Validator.PresetDuplicate",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCValidator_PresetDuplicate::RunTest(const FString& Parameters)
{
    // Both channels are builtin so CS_CHAN_UNUSED is suppressed.
    const FName ChanA("ChanA");
    const FName ChanB("ChanB");

    FCCCollisionSnapshot Snap;
    Snap.Channels.Add(MakeBuiltinChannel(ChanA, 0));
    Snap.Channels.Add(MakeBuiltinChannel(ChanB, 1));

    // Two presets: same ObjectTypeChannel, same QueryResponses → duplicate.
    auto MakeDupPreset = [&](FName Name) -> FCCPresetData
    {
        FCCPresetData P;
        P.PresetName        = Name;
        P.ObjectTypeChannel = ChanA;
        P.bIsBuiltin        = false;
        P.QueryResponses.Add(ChanA, ECR_Block);
        P.QueryResponses.Add(ChanB, ECR_Block);
        return P;
    };

    Snap.Presets.Add(MakeDupPreset("PresetAlpha"));
    Snap.Presets.Add(MakeDupPreset("PresetBeta"));
    Snap.PresetCount = Snap.Presets.Num();
    // PairwiseMatrix intentionally empty → CheckOverlapEvent returns early.

    const TArray<FCCValidationResult> Results = FCCValidator::RunAll(Snap);

    TestEqual("Exactly one result",          Results.Num(), 1);
    TestTrue("CS_PRESET_DUPLICATE fires",    HasRule(Results, "CS_PRESET_DUPLICATE"));

    const FCCValidationResult* R = FindRule(Results, "CS_PRESET_DUPLICATE");
    if (R)
    {
        TestEqual("Severity = Warning",    R->Severity, ECCValidationSeverity::Warning);
        TestEqual("Two affected presets",  R->AffectedPresets.Num(), 2);
    }
    return true;
}


// ============================================================================
//  Test 26 — CS_PRESET_ALL_IGNORE: preset ignores every channel → Warning
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCValidator_PresetAllIgnore,
    "CollisionCommander.Core.Validator.PresetAllIgnore",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCValidator_PresetAllIgnore::RunTest(const FString& Parameters)
{
    // Both channels are builtin → CS_CHAN_UNUSED is suppressed.
    // Preset is not named "NoCollision" → not exempt.
    const FName ChanA("ChanA");
    const FName ChanB("ChanB");

    FCCCollisionSnapshot Snap;
    Snap.Channels.Add(MakeBuiltinChannel(ChanA, 0));
    Snap.Channels.Add(MakeBuiltinChannel(ChanB, 1));

    FCCPresetData P;
    P.PresetName        = FName("AllIgnorePreset");
    P.ObjectTypeChannel = ChanA;
    P.bIsBuiltin        = false;
    P.QueryResponses.Add(ChanA, ECR_Ignore);
    P.QueryResponses.Add(ChanB, ECR_Ignore);
    Snap.Presets.Add(P);
    Snap.PresetCount = Snap.Presets.Num();

    const TArray<FCCValidationResult> Results = FCCValidator::RunAll(Snap);

    TestEqual("Exactly one result",           Results.Num(), 1);
    TestTrue("CS_PRESET_ALL_IGNORE fires",    HasRule(Results, "CS_PRESET_ALL_IGNORE"));

    const FCCValidationResult* R = FindRule(Results, "CS_PRESET_ALL_IGNORE");
    if (R)
    {
        TestEqual("Severity = Warning", R->Severity, ECCValidationSeverity::Warning);
    }
    return true;
}


// ============================================================================
//  Test 27 — CS_STATIC_IGNORED: dynamic preset ignores WorldStatic → Warning
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCValidator_StaticIgnored,
    "CollisionCommander.Core.Validator.StaticIgnored",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCValidator_StaticIgnored::RunTest(const FString& Parameters)
{
    // Both channels are builtin → CS_CHAN_UNUSED is suppressed.
    // Preset has ChanDynamic→Block so CS_PRESET_ALL_IGNORE does not fire.
    const FName ChanWorldStatic("WorldStatic");
    const FName ChanDynamic("DynamicChan");

    FCCCollisionSnapshot Snap;
    Snap.Channels.Add(MakeBuiltinChannel(ChanWorldStatic, 0));
    Snap.Channels.Add(MakeBuiltinChannel(ChanDynamic,     1));

    FCCPresetData P;
    P.PresetName        = FName("DynamicPreset");
    P.ObjectTypeChannel = ChanDynamic;           // not WorldStatic → rule applies
    P.bIsBuiltin        = false;
    P.QueryResponses.Add(ChanWorldStatic, ECR_Ignore); // triggers CS_STATIC_IGNORED
    P.QueryResponses.Add(ChanDynamic,     ECR_Block);  // not all-Ignore
    Snap.Presets.Add(P);
    Snap.PresetCount = Snap.Presets.Num();

    const TArray<FCCValidationResult> Results = FCCValidator::RunAll(Snap);

    TestEqual("Exactly one result",        Results.Num(), 1);
    TestTrue("CS_STATIC_IGNORED fires",    HasRule(Results, "CS_STATIC_IGNORED"));

    const FCCValidationResult* R = FindRule(Results, "CS_STATIC_IGNORED");
    if (R)
    {
        TestEqual("Severity = Warning", R->Severity, ECCValidationSeverity::Warning);
    }
    return true;
}


// ============================================================================
//  Test 28 — CS_OVERLAP_MISMATCH: A blocks B's type, B ignores A's type → Info
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCValidator_OverlapMismatch,
    "CollisionCommander.Core.Validator.OverlapMismatch",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCValidator_OverlapMismatch::RunTest(const FString& Parameters)
{
    // Both channels are builtin → CS_CHAN_UNUSED is suppressed.
    // PA blocks ChanB (PB's ObjType); PB ignores ChanA (PA's ObjType) → mismatch.
    // PA and PB have different ObjTypes → CS_PRESET_DUPLICATE does not fire.
    const FName ChanA("ChanA");
    const FName ChanB("ChanB");

    FCCCollisionSnapshot Snap;
    Snap.Channels.Add(MakeBuiltinChannel(ChanA, 0));
    Snap.Channels.Add(MakeBuiltinChannel(ChanB, 1));

    FCCPresetData PA;
    PA.PresetName        = FName("PresetA");
    PA.ObjectTypeChannel = ChanA;
    PA.bIsBuiltin        = false;
    PA.QueryResponses.Add(ChanA, ECR_Block);
    PA.QueryResponses.Add(ChanB, ECR_Block); // A blocks B's ObjectType

    FCCPresetData PB;
    PB.PresetName        = FName("PresetB");
    PB.ObjectTypeChannel = ChanB;
    PB.bIsBuiltin        = false;
    PB.QueryResponses.Add(ChanA, ECR_Ignore); // B ignores A's ObjectType → mismatch
    PB.QueryResponses.Add(ChanB, ECR_Block);

    Snap.Presets.Add(PA);
    Snap.Presets.Add(PB);
    Snap.PresetCount = Snap.Presets.Num();
    // PairwiseMatrix intentionally empty → CheckOverlapEvent returns early.

    const TArray<FCCValidationResult> Results = FCCValidator::RunAll(Snap);

    TestEqual("Exactly one result",          Results.Num(), 1);
    TestTrue("CS_OVERLAP_MISMATCH fires",    HasRule(Results, "CS_OVERLAP_MISMATCH"));

    const FCCValidationResult* R = FindRule(Results, "CS_OVERLAP_MISMATCH");
    if (R)
    {
        TestEqual("Severity = Info",       R->Severity, ECCValidationSeverity::Info);
        TestEqual("Two affected presets",  R->AffectedPresets.Num(), 2);
    }
    return true;
}


// ============================================================================
//  Test 29 — CS_NAME_CONFLICT: custom preset name matches a built-in → Error
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCValidator_NameConflict,
    "CollisionCommander.Core.Validator.NameConflict",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCValidator_NameConflict::RunTest(const FString& Parameters)
{
    // Channel is builtin → CS_CHAN_UNUSED is suppressed.
    // Built-in preset named "Pawn" and custom preset also named "Pawn".
    // Different QueryResponses so CS_PRESET_DUPLICATE does not fire.
    const FName ChanA("ChanA");

    FCCCollisionSnapshot Snap;
    Snap.Channels.Add(MakeBuiltinChannel(ChanA, 0));

    FCCPresetData PBuiltin;
    PBuiltin.PresetName        = FName("Pawn");
    PBuiltin.ObjectTypeChannel = ChanA;
    PBuiltin.bIsBuiltin        = true;
    PBuiltin.QueryResponses.Add(ChanA, ECR_Block);

    // Custom preset with same name but different response → no CS_PRESET_DUPLICATE.
    FCCPresetData PCustom;
    PCustom.PresetName        = FName("Pawn");
    PCustom.ObjectTypeChannel = ChanA;
    PCustom.bIsBuiltin        = false;
    PCustom.QueryResponses.Add(ChanA, ECR_Overlap);

    Snap.Presets.Add(PBuiltin);
    Snap.Presets.Add(PCustom);
    Snap.PresetCount = Snap.Presets.Num();
    // PairwiseMatrix intentionally empty → CheckOverlapEvent returns early.

    const TArray<FCCValidationResult> Results = FCCValidator::RunAll(Snap);

    TestEqual("Exactly one result",          Results.Num(), 1);
    TestTrue("CS_NAME_CONFLICT fires",       HasRule(Results, "CS_NAME_CONFLICT"));

    const FCCValidationResult* R = FindRule(Results, "CS_NAME_CONFLICT");
    if (R)
    {
        TestEqual("Severity = Error", R->Severity, ECCValidationSeverity::Error);
    }
    return true;
}


// ============================================================================
//  Test 30 — CS_OVERLAP_EVENT: pair resolves to Overlap in PairwiseMatrix → Warning
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCValidator_OverlapEvent,
    "CollisionCommander.Core.Validator.OverlapEvent",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCValidator_OverlapEvent::RunTest(const FString& Parameters)
{
    // Both channels are builtin → CS_CHAN_UNUSED is suppressed.
    // Presets have Overlap responses → CS_OVERLAP_MISMATCH does not fire
    // (neither side is Block while the other is Ignore).
    // PairwiseMatrix[0*2+1].QueryResult = ECR_Overlap → CS_OVERLAP_EVENT fires.
    const FName ChanA("ChanA");
    const FName ChanB("ChanB");

    FCCCollisionSnapshot Snap;
    Snap.Channels.Add(MakeBuiltinChannel(ChanA, 0));
    Snap.Channels.Add(MakeBuiltinChannel(ChanB, 1));

    FCCPresetData PA;
    PA.PresetName        = FName("PresetA");
    PA.ObjectTypeChannel = ChanA;
    PA.bIsBuiltin        = false;
    PA.QueryResponses.Add(ChanA, ECR_Overlap);
    PA.QueryResponses.Add(ChanB, ECR_Overlap);

    FCCPresetData PB;
    PB.PresetName        = FName("PresetB");
    PB.ObjectTypeChannel = ChanB;
    PB.bIsBuiltin        = false;
    PB.QueryResponses.Add(ChanA, ECR_Overlap);
    PB.QueryResponses.Add(ChanB, ECR_Overlap);

    Snap.Presets.Add(PA);
    Snap.Presets.Add(PB);
    Snap.PresetCount = 2;

    // Build the 2×2 pairwise matrix. [0*2+1] = PresetA vs PresetB = Overlap.
    FCCPairwiseResult OverlapPair;
    OverlapPair.QueryResult   = ECR_Overlap;
    OverlapPair.PhysicsResult = ECR_Overlap;

    Snap.PairwiseMatrix.SetNum(4);
    Snap.PairwiseMatrix[0] = OverlapPair; // [0,0] diagonal
    Snap.PairwiseMatrix[1] = OverlapPair; // [0,1] A vs B — CheckOverlapEvent fires here
    Snap.PairwiseMatrix[2] = OverlapPair; // [1,0] B vs A
    Snap.PairwiseMatrix[3] = OverlapPair; // [1,1] diagonal

    const TArray<FCCValidationResult> Results = FCCValidator::RunAll(Snap);

    TestEqual("Exactly one result",       Results.Num(), 1);
    TestTrue("CS_OVERLAP_EVENT fires",    HasRule(Results, "CS_OVERLAP_EVENT"));

    const FCCValidationResult* R = FindRule(Results, "CS_OVERLAP_EVENT");
    if (R)
    {
        TestEqual("Severity = Warning",    R->Severity, ECCValidationSeverity::Warning);
        TestEqual("Two affected presets",  R->AffectedPresets.Num(), 2);
    }
    return true;
}


// ============================================================================
//  Test 31 — CS_VISIBILITY_BLOCK: preset blocks Visibility channel → Info
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCValidator_VisibilityBlock,
    "CollisionCommander.Core.Validator.VisibilityBlock",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCValidator_VisibilityBlock::RunTest(const FString& Parameters)
{
    // Both channels are builtin → CS_CHAN_UNUSED is suppressed.
    // Preset has ChanA→Block so CS_PRESET_ALL_IGNORE does not fire.
    const FName ChanVisibility("Visibility");
    const FName ChanA("ChanA");

    FCCCollisionSnapshot Snap;
    Snap.Channels.Add(MakeBuiltinChannel(ChanVisibility, 0));
    Snap.Channels.Add(MakeBuiltinChannel(ChanA,          1));

    FCCPresetData P;
    P.PresetName        = FName("WeaponPreset");
    P.ObjectTypeChannel = ChanA;
    P.bIsBuiltin        = false;
    P.QueryResponses.Add(ChanVisibility, ECR_Block); // triggers CS_VISIBILITY_BLOCK
    P.QueryResponses.Add(ChanA,          ECR_Block); // not all-Ignore
    Snap.Presets.Add(P);
    Snap.PresetCount = Snap.Presets.Num();

    const TArray<FCCValidationResult> Results = FCCValidator::RunAll(Snap);

    TestEqual("Exactly one result",           Results.Num(), 1);
    TestTrue("CS_VISIBILITY_BLOCK fires",     HasRule(Results, "CS_VISIBILITY_BLOCK"));

    const FCCValidationResult* R = FindRule(Results, "CS_VISIBILITY_BLOCK");
    if (R)
    {
        TestEqual("Severity = Info", R->Severity, ECCValidationSeverity::Info);
    }
    return true;
}


// ============================================================================
//  Test 32 — Clean snapshot: a well-formed snapshot produces zero results
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCValidator_CleanSnapshot,
    "CollisionCommander.Core.Validator.CleanSnapshot",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCValidator_CleanSnapshot::RunTest(const FString& Parameters)
{
    // Two builtin channels + one custom channel (budget rules suppressed: 1 < 14).
    // Custom channel has non-Ignore responses on both presets (CS_CHAN_UNUSED suppressed).
    // No WorldStatic or Visibility channels (those rules suppressed).
    // Presets have non-Ignore, non-all-Ignore responses.
    // Different ObjectTypeChannels → no CS_PRESET_DUPLICATE.
    // Custom preset name is unique → no CS_NAME_CONFLICT.
    // PairwiseMatrix contains Block results → no CS_OVERLAP_EVENT.
    const FName ChanA("ChanA");
    const FName ChanB("ChanB");
    const FName CustomChan("MyCustomChan");

    FCCCollisionSnapshot Snap;
    Snap.Channels.Add(MakeBuiltinChannel(ChanA,     0));
    Snap.Channels.Add(MakeBuiltinChannel(ChanB,     1));
    Snap.Channels.Add(MakeCustomChannel(CustomChan, 2));

    FCCPresetData PBuiltin;
    PBuiltin.PresetName        = FName("BuiltinPreset");
    PBuiltin.ObjectTypeChannel = ChanA;
    PBuiltin.bIsBuiltin        = true;
    PBuiltin.QueryResponses.Add(ChanA,      ECR_Block);
    PBuiltin.QueryResponses.Add(ChanB,      ECR_Block);
    PBuiltin.QueryResponses.Add(CustomChan, ECR_Block);

    // Custom preset: different ObjType, different name → no duplicate, no name conflict.
    FCCPresetData PCustom;
    PCustom.PresetName        = FName("UniqueCustomPreset");
    PCustom.ObjectTypeChannel = ChanB;
    PCustom.bIsBuiltin        = false;
    PCustom.QueryResponses.Add(ChanA,      ECR_Block);
    PCustom.QueryResponses.Add(ChanB,      ECR_Block);
    PCustom.QueryResponses.Add(CustomChan, ECR_Block);

    Snap.Presets.Add(PBuiltin);
    Snap.Presets.Add(PCustom);
    Snap.PresetCount = 2;

    // 2×2 pairwise matrix — all Block (no CS_OVERLAP_EVENT).
    FCCPairwiseResult BlockPair;
    BlockPair.QueryResult   = ECR_Block;
    BlockPair.PhysicsResult = ECR_Block;
    Snap.PairwiseMatrix.SetNum(4);
    Snap.PairwiseMatrix[0] = BlockPair;
    Snap.PairwiseMatrix[1] = BlockPair;
    Snap.PairwiseMatrix[2] = BlockPair;
    Snap.PairwiseMatrix[3] = BlockPair;

    const TArray<FCCValidationResult> Results = FCCValidator::RunAll(Snap);

    TestEqual("Clean snapshot produces zero results", Results.Num(), 0);
    return true;
}


// ============================================================================
//  Test 33 — TraceChannels: channel not used as any ObjectType → bIsTraceChannel
//
//  Constructs a minimal snapshot by hand: two object-type channels used by
//  presets and one channel used by no preset. Runs the same identification
//  logic that BuildSnapshot() uses and verifies the orphan channel is flagged.
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCTraceChannelIdentification_OrphanFlagged,
    "CollisionCommander.Core.TraceChannels.OrphanFlagged",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCTraceChannelIdentification_OrphanFlagged::RunTest(const FString& Parameters)
{
    const FName ChanObjA("WorldStatic");
    const FName ChanObjB("Pawn");
    const FName ChanTrace("Visibility");

    // Build a snapshot manually with the same logic BuildSnapshot uses
    FCCCollisionSnapshot Snap;
    Snap.Channels.Add(MakeBuiltinChannel(ChanObjA,  0));
    Snap.Channels.Add(MakeBuiltinChannel(ChanObjB,  1));
    Snap.Channels.Add(MakeBuiltinChannel(ChanTrace, 2));

    // Two presets: each uses one of the object-type channels as its ObjectType
    FCCPresetData PA;
    PA.PresetName        = FName("PA");
    PA.ObjectTypeChannel = ChanObjA;
    PA.bIsBuiltin        = true;

    FCCPresetData PB;
    PB.PresetName        = FName("PB");
    PB.ObjectTypeChannel = ChanObjB;
    PB.bIsBuiltin        = false;

    Snap.Presets.Add(PA);
    Snap.Presets.Add(PB);

    // Run trace identification (mirrors BuildSnapshot logic)
    TSet<FName> ObjectTypeChannels;
    for (const FCCPresetData& P : Snap.Presets)
    {
        if (!P.ObjectTypeChannel.IsNone()) ObjectTypeChannels.Add(P.ObjectTypeChannel);
    }
    for (FCCChannelInfo& Ch : Snap.Channels)
    {
        Ch.bIsTraceChannel = !ObjectTypeChannels.Contains(Ch.Name);
        if (Ch.bIsTraceChannel) Snap.TraceChannels.Add(Ch);
    }

    TestEqual("TraceChannels has exactly one entry",       Snap.TraceChannels.Num(), 1);
    TestEqual("Visibility is the trace channel",           Snap.TraceChannels[0].Name, ChanTrace);
    TestFalse("WorldStatic is NOT a trace channel", [&]() {
        for (const FCCChannelInfo& Ch : Snap.TraceChannels) { if (Ch.Name == ChanObjA) return true; }
        return false; }());
    TestFalse("Pawn is NOT a trace channel", [&]() {
        for (const FCCChannelInfo& Ch : Snap.TraceChannels) { if (Ch.Name == ChanObjB) return true; }
        return false; }());

    return true;
}


// ============================================================================
//  Test 34 — TraceChannels: Visibility and Camera must never appear as ObjectType
//
//  Verifies the core invariant: in a snapshot where Visibility and Camera are
//  present as channels but no preset claims them as ObjectType, they land in
//  TraceChannels. This mirrors the real UE project where ECC_Visibility and
//  ECC_Camera are always trace-only channels.
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCTraceChannelIdentification_VisibilityCamera,
    "CollisionCommander.Core.TraceChannels.VisibilityAndCameraAreTrace",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCTraceChannelIdentification_VisibilityCamera::RunTest(const FString& Parameters)
{
    const FName ChanWorldStatic("WorldStatic");
    const FName ChanPawn      ("Pawn");
    const FName ChanVisibility("Visibility");
    const FName ChanCamera    ("Camera");

    FCCCollisionSnapshot Snap;
    Snap.Channels.Add(MakeBuiltinChannel(ChanWorldStatic, 0));
    Snap.Channels.Add(MakeBuiltinChannel(ChanPawn,        1));
    Snap.Channels.Add(MakeBuiltinChannel(ChanVisibility,  2));
    Snap.Channels.Add(MakeBuiltinChannel(ChanCamera,      3));

    FCCPresetData PStatic;
    PStatic.PresetName        = FName("WorldStatic");
    PStatic.ObjectTypeChannel = ChanWorldStatic;
    PStatic.bIsBuiltin        = true;

    FCCPresetData PPawn;
    PPawn.PresetName        = FName("Pawn");
    PPawn.ObjectTypeChannel = ChanPawn;
    PPawn.bIsBuiltin        = true;

    Snap.Presets.Add(PStatic);
    Snap.Presets.Add(PPawn);

    // Run trace identification
    TSet<FName> ObjectTypeChannels;
    for (const FCCPresetData& P : Snap.Presets)
    {
        if (!P.ObjectTypeChannel.IsNone()) ObjectTypeChannels.Add(P.ObjectTypeChannel);
    }
    for (FCCChannelInfo& Ch : Snap.Channels)
    {
        Ch.bIsTraceChannel = !ObjectTypeChannels.Contains(Ch.Name);
        if (Ch.bIsTraceChannel) Snap.TraceChannels.Add(Ch);
    }

    TestEqual("TraceChannels has exactly two entries (Visibility + Camera)", Snap.TraceChannels.Num(), 2);

    bool bFoundVisibility = false;
    bool bFoundCamera     = false;
    for (const FCCChannelInfo& Ch : Snap.TraceChannels)
    {
        if (Ch.Name == ChanVisibility) bFoundVisibility = true;
        if (Ch.Name == ChanCamera)     bFoundCamera     = true;
    }
    TestTrue("Visibility is in TraceChannels", bFoundVisibility);
    TestTrue("Camera is in TraceChannels",     bFoundCamera);

    return true;
}


// ============================================================================
//  Test 35 — TraceChannels: no ObjectType channel leaks into TraceChannels
//
//  Verifies the complement: no channel that is used as any preset's ObjectType
//  should appear in TraceChannels. Tests with a larger set of mixed channels.
// ============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCCTraceChannelIdentification_NoObjectTypeLeaks,
    "CollisionCommander.Core.TraceChannels.NoObjectTypeLeaks",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCCTraceChannelIdentification_NoObjectTypeLeaks::RunTest(const FString& Parameters)
{
    // Six channels: four used as ObjectTypes, two are trace-only
    const FName ObjChannels[]   = { FName("WorldStatic"), FName("Pawn"), FName("Vehicle"), FName("PhysicsBody") };
    const FName TraceChannels[] = { FName("Visibility"), FName("Camera") };

    FCCCollisionSnapshot Snap;
    for (int32 i = 0; i < 4; ++i) Snap.Channels.Add(MakeBuiltinChannel(ObjChannels[i],   i));
    for (int32 i = 0; i < 2; ++i) Snap.Channels.Add(MakeBuiltinChannel(TraceChannels[i], 4 + i));

    for (int32 i = 0; i < 4; ++i)
    {
        FCCPresetData P;
        P.PresetName        = ObjChannels[i];
        P.ObjectTypeChannel = ObjChannels[i];
        P.bIsBuiltin        = true;
        Snap.Presets.Add(P);
    }

    // Run trace identification
    TSet<FName> UsedObjectTypes;
    for (const FCCPresetData& P : Snap.Presets)
    {
        if (!P.ObjectTypeChannel.IsNone()) UsedObjectTypes.Add(P.ObjectTypeChannel);
    }
    for (FCCChannelInfo& Ch : Snap.Channels)
    {
        Ch.bIsTraceChannel = !UsedObjectTypes.Contains(Ch.Name);
        if (Ch.bIsTraceChannel) Snap.TraceChannels.Add(Ch);
    }

    // None of the ObjectType channels should appear in TraceChannels
    for (const FCCChannelInfo& TraceCh : Snap.TraceChannels)
    {
        const bool bIsObjType = UsedObjectTypes.Contains(TraceCh.Name);
        TestFalse(FString::Printf(TEXT("ObjectType channel '%s' must not be in TraceChannels"),
                                  *TraceCh.Name.ToString()), bIsObjType);
    }

    TestEqual("Exactly two trace channels identified", Snap.TraceChannels.Num(), 2);

    return true;
}
