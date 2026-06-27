// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "CCTypes.h"

class SScrollBox;
class SComboButton;

// ============================================================================
//  SCollisionActorComparePanel
//
//  Shows how a primary preset interacts with each preset in a compare list.
//  Reads data from FCCDataReader::BuildSnapshot().
//
//  Layout (outer to inner):
//    SVerticalBox
//    ├── Primary selector: label + SComboBox<TSharedPtr<FName>>
//    ├── Compare list: SBorder wrapping SScrollBox of MakeCompareRow() entries
//    └── Button row: "+ Add Preset" SComboButton | Refresh SButton
//
//  Each compare row shows the resolved response (badge), A→B, B→A, and an
//  overlap warning icon when the resolved response is ECR_Overlap.
// ============================================================================
class SCollisionActorComparePanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCollisionActorComparePanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // Called by SCollisionCommanderTab when the shared Query/Physics toggle changes.
    void SetUseQueryResponses(bool bQuery);

    // Called by Task 8 (matrix cell click) to pre-populate the comparison.
    void SetupComparison(FName Primary, FName Compare);

    // Called by SCollisionCommanderTab after each RefreshValidation().
    // Drives the CS_OVERLAP_EVENT warning icon on each compare row.
    void SetValidationResults(const TArray<FCCValidationResult>& Results);

private:
    FCCCollisionSnapshot                    Snapshot;
    TArray<FCCValidationResult>             CachedValidation;
    bool                                    bUseQueryResponses = true;
    FName                                   PrimaryPreset;
    TArray<FName>                           ComparePresets;
    TArray<TSharedPtr<FName>>               AllPresetOptions;

    TSharedPtr<SComboBox<TSharedPtr<FName>>> PrimaryCombo;
    TSharedPtr<SComboButton>                 AddComboButton;
    TSharedPtr<SScrollBox>                   CompareListContainer;

    void Refresh();
    void RebuildPresetOptions();
    void RebuildCompareList();

    TSharedRef<SWidget> MakeCompareRow(FName PresetName);
    TSharedRef<SWidget> MakeAddPresetsMenu();
    void                AddToCompare(FName PresetName);
    void                RemoveFromCompare(FName PresetName);
    int32               FindPresetIndex(FName PresetName) const;

    static FString      ResponseToString(ECollisionResponse R);
    static FLinearColor ResponseColor(ECollisionResponse R);
};
