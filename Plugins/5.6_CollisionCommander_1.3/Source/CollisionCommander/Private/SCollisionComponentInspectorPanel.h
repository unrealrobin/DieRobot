// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "CCTypes.h"

class SScrollBox;
class SComboButton;
class SHorizontalBox;
class SVerticalBox;
struct FAssetData;

// ============================================================================
//  SCollisionComponentInspectorPanel
//
//  Inspects all collision-capable primitive components on one or more selected
//  actors and shows how each interacts with a user-configured set of target
//  presets.
//
//  Layout:
//    SVerticalBox
//    ├── Actor picker list  (one SObjectPropertyEntryBox per actor + "Add Actor")
//    ├── Compare-against row  ("Compare against:" + [+] chips)
//    ├── Refresh button (right-aligned)
//    └── Table (dark-bg grid, header + per-actor section rows + component rows)
//
//  Multiple actors are supported. Each actor gets a section-header row in the
//  table followed by one row per collision-enabled component.
//
//  Grid lines match the Matrix panel: 1px slot gap reveals PanelBgColor.
//  Interaction cells use full words: Block / Overlap / Ignore.
// ============================================================================
// Fired after Refresh() completes with the latest component data.
// The tab uses this to push CS_COMPONENT_CUSTOM_PRESET results to the Validation panel.
DECLARE_DELEGATE_OneParam(FOnInspectorComponentDataRefreshed, const TArray<FCCActorComponentData>& /*ActorData*/);

class SCollisionComponentInspectorPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCollisionComponentInspectorPanel) {}
        SLATE_EVENT(FOnInspectorComponentDataRefreshed, OnComponentDataRefreshed)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetUseQueryResponses(bool bQuery);
    void Refresh();

private:
    // ── Per-actor state ────────────────────────────────────────────────────
    struct FActorEntry
    {
        TWeakObjectPtr<UObject>  Object;
        FString                  ObjectPath;   // soft path; empty = nothing selected
        FCCActorComponentData    Data;
    };

    TArray<FActorEntry>                      ActorEntries;
    FCCCollisionSnapshot                     Snapshot;
    bool                                     bUseQueryResponses = true;
    FOnInspectorComponentDataRefreshed       OnComponentDataRefreshed;
    TArray<FName>                TargetPresets;

    // ── Column model ───────────────────────────────────────────────────────
    // Each column in the interaction table is either a component (from one of
    // the loaded actors) or a manually-added preset chip. Built fresh by
    // RebuildTable() before any cells are emitted.
    struct FColumnComponent
    {
        int32   ActorEntryIndex = -1;     // index into ActorEntries; -1 = preset chip column
        FName   ComponentName;            // NAME_None for preset columns
        FString ComponentClass;           // empty for preset columns
        FName   PresetName;               // for preset columns this IS the preset
        FName   ObjectTypeChannel;
        bool    bIsCustomPreset = false;  // PresetName == "Custom"
    };

    TArray<FColumnComponent>     ColumnComponents;

    // ── Widgets ────────────────────────────────────────────────────────────
    TSharedPtr<SVerticalBox>     ActorPickerList;
    TSharedPtr<SHorizontalBox>   TargetChipsBox;
    TSharedPtr<SScrollBox>       TableContainer;   // vertical scroll for rows
    TSharedPtr<SScrollBox>       TableHScroll;     // horizontal scroll wrapping TableContainer
    TSharedPtr<SScrollBar>       TableHScrollBar;  // external H-scrollbar displayed below table
    TSharedPtr<SComboButton>     AddTargetButton;

    // ── Actor management ───────────────────────────────────────────────────
    void    AddActorEntry();
    void    RemoveActorEntry(int32 Index);
    void    OnActorPicked(const FAssetData& AssetData, int32 EntryIndex);
    AActor* ResolveActor(const FActorEntry& Entry) const;
    FString GetEntryDisplayName(const FActorEntry& Entry) const;
    void    RebuildActorPickerList();

    // ── Table ──────────────────────────────────────────────────────────────
    void RebuildTable();
    void RebuildTargetChips();

    void AddTargetPreset(FName PresetName);
    void RemoveTargetPreset(FName PresetName);

    TSharedRef<SWidget> MakeAddPresetsMenu();
    TSharedRef<SWidget> MakeColumnBannerRow() const;
    TSharedRef<SWidget> MakeHeaderRow() const;
    TSharedRef<SWidget> MakeDataRow(const FCCComponentInfo& Component, int32 RowActorIndex, bool bIsFirstOfGroup) const;
    TSharedRef<SWidget> MakeInteractionCell(FName ComponentPreset, FName TargetPreset) const;

    const FCCPresetData* FindPresetData(FName PresetName) const;
    FText BuildInteractionCellTooltip(FName ComponentPreset, FName TargetPreset) const;

    // Panel-level mouse wheel: Shift → H-scroll, else → V-scroll.
    // Needed because TableContainer uses ConsumeMouseWheel::Never so all
    // wheel events bubble up regardless of where the cursor is.
    virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

    static FText        EnabledToText(ECollisionEnabled::Type Enabled);
    static FLinearColor ResponseColor(ECollisionResponse R, bool bMissing);
    static FText        ResponseLabel(ECollisionResponse R, bool bMissing);
};
