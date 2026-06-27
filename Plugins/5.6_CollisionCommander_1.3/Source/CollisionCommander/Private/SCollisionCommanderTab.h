// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

class SCollisionMatrixPanel;
class SCollisionActorComparePanel;
class SCollisionComponentInspectorPanel;
class SCollisionExperimentPanel;
class SCollisionValidationPanel;

// Main content widget for the Collision Commander editor tab.
// Hosts a tab strip ("Matrix" | "Actor Compare") and an SWidgetSwitcher.
// MatrixPanel is stored so Task 8 can wire matrix-cell clicks to switch views.
class SCollisionCommanderTab : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCollisionCommanderTab) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    TSharedPtr<SWidgetSwitcher>                   Switcher;
    TSharedPtr<SCollisionMatrixPanel>             MatrixPanel;
    TSharedPtr<SCollisionActorComparePanel>       ActorComparePanel;
    TSharedPtr<SCollisionComponentInspectorPanel> ComponentInspectorPanel;
    TSharedPtr<SCollisionExperimentPanel>         ExperimentPanel;
    TSharedPtr<SCollisionValidationPanel>         ValidationPanel;
    int32 ActiveTabIndex     = 0;
    bool  bUseQueryResponses = true;

    ECheckBoxState GetTabCheckState(int32 TabIndex) const;
    void OnTabClicked(ECheckBoxState NewState, int32 TabIndex);

    ECheckBoxState GetQueryPhysicsState(bool bIsQuery) const;
    void OnQueryPhysicsToggle(ECheckBoxState NewState, bool bIsQuery);

    void OnMatrixCellClicked(FName RowPreset, FName ColPreset);

    // Run FCCValidator::RunAll() against a fresh snapshot and forward results
    // to ValidationPanel.  Called after commit and (Task 6) after refresh.
    void RefreshValidation();
};
