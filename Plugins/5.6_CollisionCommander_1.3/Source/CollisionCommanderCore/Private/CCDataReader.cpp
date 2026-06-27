// Copyright 2026 Paracosm. All Rights Reserved.

#include "CCDataReader.h"
#include "Engine/CollisionProfile.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"

// ============================================================================
//  BuildActorComponentData
//
//  Enumerates every UPrimitiveComponent on the given actor. Components with
//  CollisionEnabled == NoCollision are skipped — they have no collision surface
//  and are irrelevant to the inspector. For each included component, the object
//  type channel is resolved by name via UCollisionProfile so no index is stored.
// ============================================================================
FCCActorComponentData FCCDataReader::BuildActorComponentData(AActor* Actor)
{
    FCCActorComponentData Result;
    if (!Actor)
    {
        return Result;
    }

    TArray<UPrimitiveComponent*> PrimComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimComponents);

    // GetComponents only returns native C++ components on a CDO.
    // Blueprint-added (SCS) components are stored as ComponentTemplates in the
    // SimpleConstructionScript and are never registered on the CDO's OwnedComponents.
    // Walk the full Blueprint class hierarchy to collect them.
    for (UClass* C = Actor->GetClass(); C; C = C->GetSuperClass())
    {
        if (UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(C))
        {
            if (BPGC->SimpleConstructionScript)
            {
                for (USCS_Node* Node : BPGC->SimpleConstructionScript->GetAllNodes())
                {
                    if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Node->ComponentTemplate))
                    {
                        PrimComponents.AddUnique(Prim);
                    }
                }
            }
        }
    }

    UCollisionProfile* Profile = UCollisionProfile::Get();
    Result.Components.Reserve(PrimComponents.Num());

    for (UPrimitiveComponent* Component : PrimComponents)
    {
        if (!Component || Component->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
        {
            continue;
        }

        FCCComponentInfo Info;
        {
            // SCS template components have a "_GEN_VARIABLE" suffix on their FName
            // (e.g. "Box_GEN_VARIABLE"). Strip it so the display name matches what
            // the Blueprint editor shows.
            FString NameStr = Component->GetFName().ToString();
            NameStr.RemoveFromEnd(TEXT("_GEN_VARIABLE"));
            Info.ComponentName = FName(*NameStr);
        }
        Info.ComponentClass   = Component->GetClass()->GetName();
        Info.PresetName       = Component->GetCollisionProfileName();
        Info.CollisionEnabled = Component->GetCollisionEnabled();
        Info.ObjectTypeChannel = Profile
            ? Profile->ReturnChannelNameFromContainerIndex((int32)Component->GetCollisionObjectType())
            : NAME_None;

        Result.Components.Add(MoveTemp(Info));
    }

    return Result;
}

// ============================================================================
//  BuildSnapshot — top-level entry point
//
//  Flow:
//    1. Read all named channel slots from UCollisionProfile
//    2. Read all preset definitions (Query and Physics responses stored separately)
//    3. Compute the full N×N pairwise matrix for both Query and Physics
// ============================================================================
FCCCollisionSnapshot FCCDataReader::BuildSnapshot()
{
    UCollisionProfile* Profile = UCollisionProfile::Get();
    check(Profile);

    FCCCollisionSnapshot Snapshot;

    ReadChannels(Profile, Snapshot);
    ReadPresets(Profile, Snapshot, Snapshot.Presets, Snapshot.BuiltinPresetCount);

    // Identify trace channels: any channel not used as any preset's ObjectType is a
    // trace channel (Visibility, Camera, custom game trace channels, etc.).
    // Object type channels are always referenced by at least one preset's ObjectType.
    {
        TSet<FName> ObjectTypeChannels;
        ObjectTypeChannels.Reserve(Snapshot.Presets.Num());
        for (const FCCPresetData& P : Snapshot.Presets)
        {
            if (!P.ObjectTypeChannel.IsNone())
            {
                ObjectTypeChannels.Add(P.ObjectTypeChannel);
            }
        }
        for (FCCChannelInfo& Ch : Snapshot.Channels)
        {
            Ch.bIsTraceChannel = !ObjectTypeChannels.Contains(Ch.Name);
            if (Ch.bIsTraceChannel)
            {
                Snapshot.TraceChannels.Add(Ch);
            }
        }
    }

    Snapshot.PresetCount = Snapshot.Presets.Num();
    ComputePairwiseMatrix(Snapshot);

    return Snapshot;
}

// ============================================================================
//  ReadChannels
//
//  Populates the channel list in two steps:
//
//  1. Built-in channels (ECC slots 0–13): iterated directly via
//     ReturnChannelNameFromContainerIndex. These are hardcoded in the engine
//     and are always present.
//
//  2. Custom channels: sourced from DefaultChannelResponses via UE reflection.
//     DefaultChannelResponses is UPROPERTY(globalconfig) and private in C++.
//     We access it the same way CreatePreset accesses Profiles — reflection
//     bypasses C++ access control without touching private headers.
//     This gives us ONLY the channels the project actually defined in
//     Project Settings → Collision, not the phantom auto-named ECC slots
//     that ReturnChannelNameFromContainerIndex returns for undefined slots.
// ============================================================================
void FCCDataReader::ReadChannels(UCollisionProfile* Profile, FCCCollisionSnapshot& OutSnapshot)
{
    // Step 1 — built-in channels: indices 0 to UE_BUILTIN_CHANNEL_COUNT-1.
    OutSnapshot.Channels.Reserve((int32)ECC_MAX);

    for (int32 i = 0; i < UE_BUILTIN_CHANNEL_COUNT; ++i)
    {
        FName ChannelName = Profile->ReturnChannelNameFromContainerIndex(i);
        if (ChannelName == NAME_None)
        {
            continue;
        }

        // Skip unnamed ECC slots — channels that UE auto-names but the project
        // never defined in Project Settings → Collision. A named channel will
        // have a human-readable name (e.g. "Visibility", "MyWeaponTrace");
        // an unnamed slot retains the raw enum name ("EngineTraceChannel1" etc).
        {
            const FString ChStr = ChannelName.ToString();
            if (ChStr.StartsWith(TEXT("EngineTraceChannel")) ||
                ChStr.StartsWith(TEXT("GameTraceChannel")))
            {
                continue;
            }
        }

        FCCChannelInfo Info;
        Info.Name      = ChannelName;
        Info.Index     = i;
        Info.bIsBuiltin = true;
        OutSnapshot.Channels.Add(Info);
    }

    // Step 2 — custom channels via DefaultChannelResponses (reflection).
    // DefaultChannelResponses is UPROPERTY(globalconfig) in UCollisionProfile,
    // so FindFProperty resolves it even though the member is private in C++.
    FArrayProperty* DefaultChannelResponsesProp =
        FindFProperty<FArrayProperty>(UCollisionProfile::StaticClass(),
                                      TEXT("DefaultChannelResponses"));
    if (ensureMsgf(DefaultChannelResponsesProp,
        TEXT("CollisionCommander: DefaultChannelResponses not found via reflection — custom channel budget will be wrong")))
    {
        FScriptArrayHelper Helper(DefaultChannelResponsesProp,
            DefaultChannelResponsesProp->ContainerPtrToValuePtr<void>(Profile));

        for (int32 i = 0; i < Helper.Num(); ++i)
        {
            const FCustomChannelSetup* Setup =
                reinterpret_cast<const FCustomChannelSetup*>(Helper.GetRawPtr(i));
            if (!Setup || Setup->Name == NAME_None)
            {
                continue;
            }

            FCCChannelInfo Info;
            Info.Name      = Setup->Name;
            Info.Index     = (int32)Setup->Channel.GetValue();
            Info.bIsBuiltin = false;
            OutSnapshot.Channels.Add(Info);
        }
    }
}

// ============================================================================
//  ReadPresets
//
//  Reads every collision preset via GetProfileByIndex. For each preset:
//    - ObjectTypeChannel is resolved by name from the template's ObjectType enum
//    - QueryResponses and PhysicsResponses are populated from the preset's
//      ResponseToChannels container, gated by the CollisionEnabled flag:
//        QueryOnly or QueryAndPhysics    → Query responses are the actual values
//        PhysicsOnly or QueryAndPhysics  → Physics responses are the actual values
//        Otherwise (NoCollision, Probe)  → that response type is all ECR_Ignore
//
//  Built-in presets (engine defaults) have bCanModify=false. Custom (project)
//  presets have bCanModify=true. UE always stores built-ins first in the profile
//  list, so BuiltinPresetCount is incremented for each leading !bCanModify preset.
// ============================================================================
void FCCDataReader::ReadPresets(UCollisionProfile* Profile, const FCCCollisionSnapshot& Snapshot,
                                TArray<FCCPresetData>& OutPresets, int32& OutBuiltinPresetCount)
{
    const int32 NumProfiles = Profile->GetNumOfProfiles();
    OutPresets.Reserve(NumProfiles);
    OutBuiltinPresetCount = 0;

    for (int32 i = 0; i < NumProfiles; ++i)
    {
        const FCollisionResponseTemplate* Template = Profile->GetProfileByIndex(i);
        if (!Template)
        {
            continue;
        }

        FCCPresetData Preset;
        Preset.PresetName        = Template->Name;
        Preset.bIsBuiltin        = !Template->bCanModify;
        Preset.Description       = Template->HelpMessage;
        Preset.CollisionEnabled  = Template->CollisionEnabled.GetValue();

        // Resolve the object type channel by name using its enum index
        Preset.ObjectTypeChannel = Profile->ReturnChannelNameFromContainerIndex(
            (int32)Template->ObjectType.GetValue());

        // Determine which response types this preset participates in.
        // ProbeOnly and QueryAndProbe are UE5 additions — probes are query-side only.
        const ECollisionEnabled::Type Enabled = Template->CollisionEnabled.GetValue();
        const bool bHasQuery   = (Enabled == ECollisionEnabled::QueryOnly
                                || Enabled == ECollisionEnabled::QueryAndPhysics
                                || Enabled == ECollisionEnabled::ProbeOnly
                                || Enabled == ECollisionEnabled::QueryAndProbe);
        const bool bHasPhysics = (Enabled == ECollisionEnabled::PhysicsOnly
                                || Enabled == ECollisionEnabled::QueryAndPhysics);

        // Populate per-channel response maps keyed by channel name (never by index)
        for (const FCCChannelInfo& Channel : Snapshot.Channels)
        {
            ECollisionChannel ChannelEnum = (ECollisionChannel)Channel.Index;
            ECollisionResponse RawResponse = Template->ResponseToChannels.GetResponse(ChannelEnum);

            Preset.QueryResponses.Add(  Channel.Name, bHasQuery   ? RawResponse : ECR_Ignore);
            Preset.PhysicsResponses.Add(Channel.Name, bHasPhysics ? RawResponse : ECR_Ignore);
        }

        // Count leading built-in presets (UE always stores them first)
        if (Preset.bIsBuiltin)
        {
            OutBuiltinPresetCount++;
        }

        OutPresets.Add(MoveTemp(Preset));
    }
}

// ============================================================================
//  ComputePairwiseMatrix
//
//  Fills the flat N×N matrix. For every ordered pair (A, B), resolves both
//  the Query and Physics interaction using the UE min() rule and stores the
//  result in PairwiseMatrix[A * N + B].
// ============================================================================
void FCCDataReader::ComputePairwiseMatrix(FCCCollisionSnapshot& OutSnapshot)
{
    const int32 N = OutSnapshot.PresetCount;
    OutSnapshot.PairwiseMatrix.SetNum(N * N);

    for (int32 A = 0; A < N; ++A)
    {
        for (int32 B = 0; B < N; ++B)
        {
            FCCPairwiseResult& Cell = OutSnapshot.PairwiseMatrix[A * N + B];
            Cell.QueryResult   = ResolvePair(OutSnapshot.Presets[A], OutSnapshot.Presets[B], true);
            Cell.PhysicsResult = ResolvePair(OutSnapshot.Presets[A], OutSnapshot.Presets[B], false);
        }
    }
}

// ============================================================================
//  ComputeExperimentInteractions
//
//  Treats FCCExperimentPreset as a temporary FCCPresetData and runs ResolvePair
//  against every preset in the snapshot. Returns one FCCPairwiseResult per
//  preset, index-aligned with Snapshot.Presets.
// ============================================================================
TArray<FCCPairwiseResult> FCCDataReader::ComputeExperimentInteractions(
    const FCCExperimentPreset& Exp,
    const FCCCollisionSnapshot& Snapshot)
{
    // Construct a temporary FCCPresetData so we can reuse the existing ResolvePair.
    FCCPresetData TempPreset;
    TempPreset.PresetName        = Exp.PresetName;
    TempPreset.ObjectTypeChannel = Exp.ObjectTypeChannel;
    TempPreset.QueryResponses    = Exp.QueryResponses;
    TempPreset.PhysicsResponses  = Exp.PhysicsResponses;
    TempPreset.bIsBuiltin        = false;

    TArray<FCCPairwiseResult> Results;
    Results.Reserve(Snapshot.Presets.Num());

    for (const FCCPresetData& Existing : Snapshot.Presets)
    {
        FCCPairwiseResult Cell;
        Cell.QueryResult   = ResolvePair(TempPreset, Existing, true);
        Cell.PhysicsResult = ResolvePair(TempPreset, Existing, false);
        Results.Add(Cell);
    }

    return Results;
}


// ============================================================================
//  PresetNameExists
//
//  Returns true if a preset with the given name already exists in
//  UCollisionProfile. Returns false for empty names or if the profile is not
//  available (e.g. called outside editor context).
// ============================================================================
bool FCCDataReader::PresetNameExists(FName Name)
{
    if (Name.IsNone()) return false;

    UCollisionProfile* Profile = UCollisionProfile::Get();
    if (!Profile) return false;

    const int32 NumProfiles = Profile->GetNumOfProfiles();
    for (int32 i = 0; i < NumProfiles; ++i)
    {
        const FCollisionResponseTemplate* T = Profile->GetProfileByIndex(i);
        if (T && T->Name == Name)
        {
            return true;
        }
    }
    return false;
}

// ============================================================================
//  CreatePreset
//
//  Writes ExperimentPreset to UCollisionProfile as a new (or overwritten)
//  collision profile.
//
//  Flow:
//    1. Validate: PresetName is non-empty.
//    2. Check for duplicate name.
//         bAllowOverwrite=false → log + return false.
//         bAllowOverwrite=true  → overwrite in-place via reflection.
//    3. Resolve ObjectTypeChannel to ECollisionChannel via reverse lookup.
//    4. Build a FCollisionResponseTemplate:
//         - CollisionEnabled  = QueryAndPhysics (sensible default)
//         - bCanModify        = true (custom user preset)
//         - ResponseToChannels populated from Exp.QueryResponses (canonical;
//           UE stores a single response per channel, not split Query/Physics)
//    5. Append (or overwrite) via UPROPERTY reflection and call SaveConfig().
// ============================================================================
bool FCCDataReader::CreatePreset(const FCCExperimentPreset& Exp, bool bAllowOverwrite)
{
    // 1. Validate non-empty name.
    if (Exp.PresetName.IsNone() || Exp.PresetName.ToString().IsEmpty())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("CollisionCommander: CreatePreset failed — preset name is empty."));
        return false;
    }

    UCollisionProfile* Profile = UCollisionProfile::Get();
    if (!Profile)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("CollisionCommander: CreatePreset failed — UCollisionProfile unavailable."));
        return false;
    }

    // Helper: resolve a channel name to its ECollisionChannel container index.
    // ReturnContainerIndexFromChannelName is not exported from the Engine DLL in
    // all build configurations, so we reverse-lookup via the working
    // ReturnChannelNameFromContainerIndex instead.
    auto FindChannelIndex = [&Profile](FName ChannelName) -> int32
    {
        for (int32 i = 0; i < (int32)ECC_MAX; ++i)
        {
            if (Profile->ReturnChannelNameFromContainerIndex(i) == ChannelName)
            {
                return i;
            }
        }
        return INDEX_NONE;
    };

    // 2. Check for duplicate name.
    bool bExistingFound = false;
    const int32 NumProfiles = Profile->GetNumOfProfiles();
    for (int32 i = 0; i < NumProfiles; ++i)
    {
        const FCollisionResponseTemplate* Existing = Profile->GetProfileByIndex(i);
        if (Existing && Existing->Name == Exp.PresetName)
        {
            if (!bAllowOverwrite)
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("CollisionCommander: CreatePreset failed — preset '%s' already exists."),
                    *Exp.PresetName.ToString());
                return false;
            }
            bExistingFound = true;
            break;
        }
    }

    // 3. Resolve ObjectTypeChannel name to ECollisionChannel container index.
    const int32 ObjTypeIndex = FindChannelIndex(Exp.ObjectTypeChannel);
    if (ObjTypeIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("CollisionCommander: CreatePreset failed — ObjectTypeChannel '%s' not found."),
            *Exp.ObjectTypeChannel.ToString());
        return false;
    }

    // 4. Build the new profile template.
    FCollisionResponseTemplate NewTemplate;
    NewTemplate.Name             = Exp.PresetName;
    NewTemplate.HelpMessage      = Exp.Description;
    NewTemplate.CollisionEnabled = Exp.CollisionEnabled.GetValue();
    NewTemplate.bCanModify       = true;
    NewTemplate.ObjectType       = (ECollisionChannel)ObjTypeIndex;

    // Populate per-channel responses from QueryResponses (canonical value).
    // UE's FCollisionResponseTemplate stores a single ResponseToChannels map —
    // it does not split Query vs Physics. QueryResponses is used as the source
    // per the spec (Task 5 note).
    for (const auto& KVP : Exp.QueryResponses)
    {
        const int32 ChIndex = FindChannelIndex(KVP.Key);
        if (ChIndex != INDEX_NONE)
        {
            NewTemplate.ResponseToChannels.SetResponse((ECollisionChannel)ChIndex, KVP.Value);
        }
    }

    // 5. Access Profiles via UPROPERTY reflection.
    //    UCollisionProfile::Profiles is private in C++ (UE 5.7+) but is a
    //    config UPROPERTY accessible through UE's reflection system.
    FArrayProperty* ProfilesProp = FindFProperty<FArrayProperty>(
        UCollisionProfile::StaticClass(), TEXT("Profiles"));
    if (!ProfilesProp)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("CollisionCommander: CreatePreset failed — could not locate Profiles property via reflection."));
        return false;
    }

    FScriptArrayHelper ProfilesHelper(ProfilesProp,
        ProfilesProp->ContainerPtrToValuePtr<void>(Profile));

    // Overwrite existing entry in-place when allowed.
    if (bAllowOverwrite && bExistingFound)
    {
        for (int32 i = 0; i < ProfilesHelper.Num(); ++i)
        {
            FCollisionResponseTemplate* Entry =
                reinterpret_cast<FCollisionResponseTemplate*>(ProfilesHelper.GetRawPtr(i));
            if (Entry && Entry->Name == Exp.PresetName)
            {
                *Entry = NewTemplate;
                Profile->SaveConfig();
                UE_LOG(LogTemp, Log,
                    TEXT("CollisionCommander: Preset '%s' updated successfully."),
                    *Exp.PresetName.ToString());
                return true;
            }
        }
        // Shouldn't happen if GetNumOfProfiles was consistent with the array.
        UE_LOG(LogTemp, Warning,
            TEXT("CollisionCommander: CreatePreset overwrite failed — entry '%s' not found in array."),
            *Exp.PresetName.ToString());
        return false;
    }

    // Append new entry.
    const int32 NewIdx = ProfilesHelper.AddValue();
    FCollisionResponseTemplate* NewEntry =
        reinterpret_cast<FCollisionResponseTemplate*>(ProfilesHelper.GetRawPtr(NewIdx));
    *NewEntry = NewTemplate;

    Profile->SaveConfig();

    UE_LOG(LogTemp, Log,
        TEXT("CollisionCommander: Preset '%s' created successfully."),
        *Exp.PresetName.ToString());
    return true;
}


// ============================================================================
//  ResolvePair
//
//  UE collision resolution rule (must be exact per spec):
//    result = min(A's response to B's ObjectType, B's response to A's ObjectType)
//    ECR_Ignore=0 < ECR_Overlap=1 < ECR_Block=2
//
//  If either preset has no entry for the other's object type channel, the
//  result is Ignore (safe default — no interaction is better than wrong interaction).
// ============================================================================
ECollisionResponse FCCDataReader::ResolvePair(const FCCPresetData& A, const FCCPresetData& B,
                                              bool bUseQueryResponses)
{
    const TMap<FName, ECollisionResponse>& ResponsesA = bUseQueryResponses
        ? A.QueryResponses : A.PhysicsResponses;
    const TMap<FName, ECollisionResponse>& ResponsesB = bUseQueryResponses
        ? B.QueryResponses : B.PhysicsResponses;

    const ECollisionResponse* AtoB = ResponsesA.Find(B.ObjectTypeChannel);
    const ECollisionResponse* BtoA = ResponsesB.Find(A.ObjectTypeChannel);

    if (!AtoB || !BtoA)
    {
        return ECR_Ignore;
    }

    return (ECollisionResponse)FMath::Min((int32)*AtoB, (int32)*BtoA);
}
