// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "CCTypes.h"

class SComboButton;
class SEditableText;
class SBox;

// ============================================================================
//  SCollisionExperimentPanel
//
//  The Experiment tab panel. Lets the user design a collision preset from
//  scratch (or using an existing preset as a starting point), then preview
//  how it would interact with every existing preset in the project.
//
//  Layout (top to bottom):
//    1. Load-from-existing combo — pre-fills all fields from a saved preset
//    2. Preset identity — editable name + object type channel dropdown
//    3. Channel responses — per-channel Ignore/Overlap/Block selectors
//       grouped by Game / Engine channels, 3 columns wide
//    4. Interaction table — live preview of results vs all existing presets
//       fixed 200/200/72 column layout, matrix-style result cells
//    5. Button bar — "Create Preset" button
//
//  OnExperimentChanged fires whenever any field changes. The tab subscribes
//  and passes the new experiment state to ComputeExperimentInteractions.
//  OnCreateRequested fires when the user presses "Create Preset"; the tab
//  owns the write logic.
// ============================================================================
class SCollisionExperimentPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCollisionExperimentPanel) {}
        SLATE_EVENT(FSimpleDelegate, OnExperimentChanged)
        SLATE_EVENT(FSimpleDelegate, OnCreateRequested)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // Re-reads UCollisionProfile and rebuilds options. Preserves existing
    // ExperimentPreset state where possible (preset names still valid).
    void Refresh();

    // Called by SCollisionCommanderTab on the shared Query/Physics toggle.
    // Updates which response map is shown in the channel grid.
    void SetUseQueryResponses(bool bQuery);

    // Returns the current in-memory experiment preset (read-only).
    const FCCExperimentPreset& GetExperimentPreset() const { return ExperimentPreset; }

    // Returns the preset name that was loaded via the source combo, or NAME_None
    // if the user is working on a brand-new preset. Used by the tab to detect
    // the overwrite-existing case when "Create Preset" is clicked.
    FName GetLoadedFromPresetName() const { return LoadedFromPresetName; }

private:
    // ── Data ────────────────────────────────────────────────────────────────
    FCCCollisionSnapshot  BaseSnapshot;
    FCCExperimentPreset   ExperimentPreset;
    bool                  bUseQueryResponses = true;

    // Tracks which preset was loaded via the source combo. NAME_None when the
    // user is working on a new preset (sentinel selected).
    FName LoadedFromPresetName = NAME_None;

    // Delegates wired by SCollisionCommanderTab
    FSimpleDelegate OnExperimentChanged;
    FSimpleDelegate OnCreateRequested;

    // ── Combo box data sources ───────────────────────────────────────────────
    // AllPresetOptions[0] is the "New Preset" sentinel; rest are snapshot presets.
    TArray<TSharedPtr<FName>> AllPresetOptions;
    TArray<TSharedPtr<FName>> AllChannelOptions;  // all named channels

    // ── Widgets that need programmatic updates ───────────────────────────────
    TSharedPtr<SComboBox<TSharedPtr<FName>>> SourceCombo;
    TSharedPtr<SComboBox<TSharedPtr<FName>>> ObjectTypeCombo;
    TSharedPtr<SEditableText>                NameField;
    TSharedPtr<SEditableText>                DescriptionField;

    TArray<TSharedPtr<ECollisionEnabled::Type>>              CollisionEnabledOptions;
    TSharedPtr<SComboBox<TSharedPtr<ECollisionEnabled::Type>>> CollisionEnabledCombo;

    // Channel response area — replaced entirely on every RebuildChannelGrid().
    TSharedPtr<SBox> ChannelContentBox;

    // Interaction preview area — replaced on every RebuildInteractionTable().
    TSharedPtr<SBox> InteractionAreaBox;

    // Guards against OnExperimentChanged firing during programmatic combo updates.
    bool bSuppressCallbacks = false;

    // ── Rebuild helpers ──────────────────────────────────────────────────────
    void RebuildOptions();          // rebuilds AllPresetOptions and AllChannelOptions
    void RebuildChannelGrid();      // replaces ChannelContentBox content
    void RebuildInteractionTable(); // replaces InteractionAreaBox content

    // Calls RebuildInteractionTable() then fires OnExperimentChanged to the tab.
    // All collision-affecting edits (object type, channel responses, load) go here.
    // Name edits do NOT go here — they don't affect collision computation.
    void OnCollisionDataChanged();

    // ── Cell / menu factories ────────────────────────────────────────────────
    // One row entry (formatted label + response combo) for a single channel.
    TSharedRef<SWidget> MakeChannelEntry(FName ChannelName);

    // Popup menu with Ignore / Overlap / Block buttons for a channel combo.
    TSharedRef<SWidget> MakeResponseMenu(FName ChannelName);

    // ── Actions ─────────────────────────────────────────────────────────────
    void LoadFromSelected(TSharedPtr<FName> SelectedItem);
    void SetChannelResponse(FName ChannelName, ECollisionResponse R);

    // ── Utilities ────────────────────────────────────────────────────────────

    // Builds the tooltip text for a result cell in the interaction table,
    // showing the per-side responses and the resolved result.
    static FText        BuildResultTooltip(const FCCExperimentPreset& Exp,
                                           const FCCPresetData& Existing,
                                           ECollisionResponse Resolved,
                                           bool bQuery);

    static FString      ResponseToString(ECollisionResponse R);
    static FLinearColor ResponseColor(ECollisionResponse R);
};
