// Copyright 2026 Paracosm. All Rights Reserved.

#include "SCollisionCommanderTab.h"
#include "SCollisionMatrixPanel.h"
#include "SCollisionActorComparePanel.h"
#include "SCollisionComponentInspectorPanel.h"
#include "SCollisionExperimentPanel.h"
#include "SCollisionValidationPanel.h"
#include "FCCValidator.h"
#include "UCollisionCommanderSettings.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/MessageDialog.h"
#include "Styling/AppStyle.h"
#include "CCDataReader.h"

#define LOCTEXT_NAMESPACE "CollisionCommander"

void SCollisionCommanderTab::Construct(const FArguments& InArgs)
{
    ChildSlot
    [
        SNew(SVerticalBox)

        // ── Tab strip ──────────────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(FMargin(4.f, 2.f))
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SCheckBox)
                    .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                    .IsChecked(this, &SCollisionCommanderTab::GetTabCheckState, 0)
                    .OnCheckStateChanged(this, &SCollisionCommanderTab::OnTabClicked, 0)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("MatrixTab", "Matrix"))
                        .Margin(FMargin(8.f, 2.f))
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(4.f, 0.f, 0.f, 0.f))
                [
                    SNew(SCheckBox)
                    .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                    .IsChecked(this, &SCollisionCommanderTab::GetTabCheckState, 1)
                    .OnCheckStateChanged(this, &SCollisionCommanderTab::OnTabClicked, 1)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("ActorCompareTab", "Collision Compare"))
                        .Margin(FMargin(8.f, 2.f))
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(4.f, 0.f, 0.f, 0.f))
                [
                    SNew(SCheckBox)
                    .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                    .IsChecked(this, &SCollisionCommanderTab::GetTabCheckState, 2)
                    .OnCheckStateChanged(this, &SCollisionCommanderTab::OnTabClicked, 2)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("ComponentInspectorTab", "Actor Inspector"))
                        .Margin(FMargin(8.f, 2.f))
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(4.f, 0.f, 0.f, 0.f))
                [
                    SNew(SCheckBox)
                    .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                    .IsChecked(this, &SCollisionCommanderTab::GetTabCheckState, 3)
                    .OnCheckStateChanged(this, &SCollisionCommanderTab::OnTabClicked, 3)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("ExperimentTab", "Experiment"))
                        .Margin(FMargin(8.f, 2.f))
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(4.f, 0.f, 0.f, 0.f))
                [
                    SNew(SCheckBox)
                    .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                    .IsChecked(this, &SCollisionCommanderTab::GetTabCheckState, 4)
                    .OnCheckStateChanged(this, &SCollisionCommanderTab::OnTabClicked, 4)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("ValidationTab", "Validation"))
                        .Margin(FMargin(8.f, 2.f))
                    ]
                ]

                + SHorizontalBox::Slot()
                .FillWidth(1.f)

                // ── Validation summary badge ─────────────────────────────────
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(FMargin(4.f, 0.f, 8.f, 0.f))
                [
                    SNew(SButton)
                    .ButtonStyle(FAppStyle::Get(), "FlatButton")
                    .ToolTipText(LOCTEXT("ValidationBadgeTip",
                        "Validation summary.\nClick to open the Validation tab."))
                    .OnClicked_Lambda([this]()
                    {
                        if (ActiveTabIndex != 4)
                        {
                            ActiveTabIndex = 4;
                            Switcher->SetActiveWidgetIndex(4);
                        }
                        return FReply::Handled();
                    })
                    [
                        SNew(SHorizontalBox)

                        // ✓ — shown when no issues
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(TEXT("\u2713")))
                            .ColorAndOpacity(FLinearColor(0.20f, 0.75f, 0.20f, 1.f))
                            .Visibility(TAttribute<EVisibility>::CreateLambda([this]() -> EVisibility
                            {
                                return (ValidationPanel.IsValid() && !ValidationPanel->HasResults())
                                    ? EVisibility::Visible : EVisibility::Collapsed;
                            }))
                        ]

                        // ✕ N — errors
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(STextBlock)
                            .Text(TAttribute<FText>::CreateLambda([this]() -> FText
                            {
                                const int32 N = ValidationPanel.IsValid()
                                    ? ValidationPanel->GetResultCount(ECCValidationSeverity::Error) : 0;
                                return FText::Format(LOCTEXT("BadgeErrorFmt", "\u2715 {0}"), FText::AsNumber(N));
                            }))
                            .ColorAndOpacity(FLinearColor(0.90f, 0.15f, 0.15f, 1.f))
                            .Visibility(TAttribute<EVisibility>::CreateLambda([this]() -> EVisibility
                            {
                                return (ValidationPanel.IsValid() &&
                                    ValidationPanel->GetResultCount(ECCValidationSeverity::Error) > 0)
                                    ? EVisibility::Visible : EVisibility::Collapsed;
                            }))
                        ]

                        // ⚠ M — warnings
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(FMargin(4.f, 0.f, 0.f, 0.f))
                        [
                            SNew(STextBlock)
                            .Text(TAttribute<FText>::CreateLambda([this]() -> FText
                            {
                                const int32 N = ValidationPanel.IsValid()
                                    ? ValidationPanel->GetResultCount(ECCValidationSeverity::Warning) : 0;
                                return FText::Format(LOCTEXT("BadgeWarnFmt", "\u26A0 {0}"), FText::AsNumber(N));
                            }))
                            .ColorAndOpacity(FLinearColor(0.90f, 0.75f, 0.10f, 1.f))
                            .Visibility(TAttribute<EVisibility>::CreateLambda([this]() -> EVisibility
                            {
                                return (ValidationPanel.IsValid() &&
                                    ValidationPanel->GetResultCount(ECCValidationSeverity::Warning) > 0)
                                    ? EVisibility::Visible : EVisibility::Collapsed;
                            }))
                        ]

                        // i K — info
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(FMargin(4.f, 0.f, 0.f, 0.f))
                        [
                            SNew(STextBlock)
                            .Text(TAttribute<FText>::CreateLambda([this]() -> FText
                            {
                                const int32 N = ValidationPanel.IsValid()
                                    ? ValidationPanel->GetResultCount(ECCValidationSeverity::Info) : 0;
                                return FText::Format(LOCTEXT("BadgeInfoFmt", "i {0}"), FText::AsNumber(N));
                            }))
                            .ColorAndOpacity(FLinearColor(0.55f, 0.55f, 0.55f, 1.f))
                            .Visibility(TAttribute<EVisibility>::CreateLambda([this]() -> EVisibility
                            {
                                return (ValidationPanel.IsValid() &&
                                    ValidationPanel->GetResultCount(ECCValidationSeverity::Info) > 0)
                                    ? EVisibility::Visible : EVisibility::Collapsed;
                            }))
                        ]
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(8.f, 0.f, 2.f, 0.f))
                [
                    SNew(SCheckBox)
                    .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                    .IsChecked(this, &SCollisionCommanderTab::GetQueryPhysicsState, true)
                    .OnCheckStateChanged(this, &SCollisionCommanderTab::OnQueryPhysicsToggle, true)
                    .ToolTipText(LOCTEXT("QueryToggleTip",
                        "Query mode\n"
                        "Shows how presets interact during raycasts, sweeps, and overlap events.\n"
                        "Presets with Collision Enabled = Physics Only appear as Ignore in this view."))
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("QueryToggle", "Query"))
                        .Margin(FMargin(6.f, 2.f))
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(2.f, 0.f, 0.f, 0.f))
                [
                    SNew(SCheckBox)
                    .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                    .IsChecked(this, &SCollisionCommanderTab::GetQueryPhysicsState, false)
                    .OnCheckStateChanged(this, &SCollisionCommanderTab::OnQueryPhysicsToggle, false)
                    .ToolTipText(LOCTEXT("PhysicsToggleTip",
                        "Physics mode\n"
                        "Shows how presets interact during physics simulation (collisions, constraints).\n"
                        "Presets with Collision Enabled = Query Only appear as Ignore in this view."))
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("PhysicsToggle", "Physics"))
                        .Margin(FMargin(6.f, 2.f))
                    ]
                ]
            ]
        ]

        // ── Content area ───────────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .FillHeight(1.f)
        [
            SAssignNew(Switcher, SWidgetSwitcher)
            .WidgetIndex(0)

            + SWidgetSwitcher::Slot()
            [
                SAssignNew(MatrixPanel, SCollisionMatrixPanel)
                .OnCellClicked(this, &SCollisionCommanderTab::OnMatrixCellClicked)
            ]

            + SWidgetSwitcher::Slot()
            [
                SAssignNew(ActorComparePanel, SCollisionActorComparePanel)
            ]

            + SWidgetSwitcher::Slot()
            [
                SAssignNew(ComponentInspectorPanel, SCollisionComponentInspectorPanel)
                .OnComponentDataRefreshed_Lambda([this](const TArray<FCCActorComponentData>& ActorData)
                {
                    if (ValidationPanel.IsValid())
                    {
                        ValidationPanel->SetComponentResults(
                            FCCValidator::RunComponentChecks(ActorData));
                    }
                })
            ]

            + SWidgetSwitcher::Slot()
            [
                SAssignNew(ExperimentPanel, SCollisionExperimentPanel)
                .OnExperimentChanged_Lambda([this]()
                {
                    // Interaction table is self-contained; no-op here.
                    // Reserved for future Matrix sync if needed.
                })
                .OnCreateRequested_Lambda([this]()
                {
                    if (!ExperimentPanel.IsValid())
                    {
                        return;
                    }

                    const FCCExperimentPreset& Exp = ExperimentPanel->GetExperimentPreset();

                    // ── Case 1: Empty name ─────────────────────────────────────────────
                    if (Exp.PresetName.IsNone() || Exp.PresetName.ToString().IsEmpty())
                    {
                        FMessageDialog::Open(EAppMsgType::Ok,
                            LOCTEXT("EmptyPresetNameError",
                                "Please enter a preset name before creating.\n\n"
                                "The preset name cannot be empty."));
                        return;
                    }

                    const FName LoadedFrom = ExperimentPanel->GetLoadedFromPresetName();

                    // ── Case 2: Same name as the preset we loaded from → overwrite ────
                    // The user loaded an existing preset, made changes, and is about to
                    // write back with the same name. Warn that this will overwrite a
                    // preset that may already be in use by actors in the project.
                    if (!LoadedFrom.IsNone() && Exp.PresetName == LoadedFrom)
                    {
                        const EAppReturnType::Type Confirm = FMessageDialog::Open(
                            EAppMsgType::YesNo,
                            FText::Format(
                                LOCTEXT("OverwritePresetConfirmFmt",
                                    "'{0}' was loaded from an existing preset.\n\n"
                                    "Clicking Yes will OVERWRITE the existing preset '{0}' "
                                    "in Project Settings.\n\n"
                                    "Any actors currently using this preset will be affected "
                                    "immediately. Make sure this is intentional.\n\n"
                                    "Do you want to overwrite it?"),
                                FText::FromName(Exp.PresetName)));

                        if (Confirm != EAppReturnType::Yes)
                        {
                            return;
                        }

                        if (FCCDataReader::CreatePreset(Exp, /*bAllowOverwrite=*/true))
                        {
                            if (ExperimentPanel.IsValid()) { ExperimentPanel->Refresh(); }
                            if (MatrixPanel.IsValid())     { MatrixPanel->Refresh(); }

                            FNotificationInfo Info(FText::Format(
                                LOCTEXT("OverwritePresetSuccessFmt",
                                    "Preset '{0}' updated successfully."),
                                FText::FromName(Exp.PresetName)));
                            Info.bFireAndForget = true;
                            Info.ExpireDuration = 3.f;
                            FSlateNotificationManager::Get().AddNotification(Info);
                        }
                        else
                        {
                            FMessageDialog::Open(EAppMsgType::Ok,
                                FText::Format(
                                    LOCTEXT("UpdatePresetFailedFmt",
                                        "Failed to update preset '{0}'.\n\n"
                                        "Check the Output Log for details."),
                                    FText::FromName(Exp.PresetName)));
                        }
                        return;
                    }

                    // ── Case 3: Name already exists but is NOT the loaded-from preset ─
                    // This is a name collision with a different preset. Block creation
                    // and ask the user to choose a different name.
                    if (FCCDataReader::PresetNameExists(Exp.PresetName))
                    {
                        FMessageDialog::Open(EAppMsgType::Ok,
                            FText::Format(
                                LOCTEXT("DuplicatePresetNameError",
                                    "A preset named '{0}' already exists.\n\n"
                                    "Please choose a different name."),
                                FText::FromName(Exp.PresetName)));
                        return;
                    }

                    // ── Case 4: New preset — normal create flow ────────────────────────
                    const EAppReturnType::Type Confirm = FMessageDialog::Open(
                        EAppMsgType::YesNo,
                        FText::Format(
                            LOCTEXT("CreatePresetConfirmFmt",
                                "Create preset '{0}' in Project Settings?\n\n"
                                "This will be saved to your project's collision configuration."),
                            FText::FromName(Exp.PresetName)));

                    if (Confirm != EAppReturnType::Yes)
                    {
                        return;
                    }

                    if (FCCDataReader::CreatePreset(Exp))
                    {
                        if (ExperimentPanel.IsValid()) { ExperimentPanel->Refresh(); }
                        if (MatrixPanel.IsValid())     { MatrixPanel->Refresh(); }

                        FNotificationInfo Info(FText::Format(
                            LOCTEXT("CreatePresetSuccessFmt",
                                "Preset '{0}' created successfully."),
                            FText::FromName(Exp.PresetName)));
                        Info.bFireAndForget = true;
                        Info.ExpireDuration = 3.f;
                        FSlateNotificationManager::Get().AddNotification(Info);
                    }
                    else
                    {
                        FMessageDialog::Open(EAppMsgType::Ok,
                            FText::Format(
                                LOCTEXT("CreatePresetFailedFmt",
                                    "Failed to create preset '{0}'.\n\n"
                                    "Check the Output Log for details."),
                                FText::FromName(Exp.PresetName)));
                    }
                })
            ]

            // ── Tab 4: Validation ─────────────────────────────────────────────
            + SWidgetSwitcher::Slot()
            [
                SAssignNew(ValidationPanel, SCollisionValidationPanel)
                .OnNavigateToTab_Lambda([this](int32 TabIndex)
                {
                    if (TabIndex != ActiveTabIndex)
                    {
                        ActiveTabIndex = TabIndex;
                        Switcher->SetActiveWidgetIndex(TabIndex);
                    }
                })
                .OnRunValidationRequested_Lambda([this]()
                {
                    RefreshValidation();
                })
            ]
        ]
    ];

    // Populate validation results and the summary badge on first display.
    RefreshValidation();
}

ECheckBoxState SCollisionCommanderTab::GetTabCheckState(int32 TabIndex) const
{
    return (ActiveTabIndex == TabIndex) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SCollisionCommanderTab::OnTabClicked(ECheckBoxState /*NewState*/, int32 TabIndex)
{
    if (ActiveTabIndex != TabIndex)
    {
        ActiveTabIndex = TabIndex;
        Switcher->SetActiveWidgetIndex(TabIndex);
    }
}

ECheckBoxState SCollisionCommanderTab::GetQueryPhysicsState(bool bIsQuery) const
{
    return (bUseQueryResponses == bIsQuery) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SCollisionCommanderTab::OnQueryPhysicsToggle(ECheckBoxState /*NewState*/, bool bIsQuery)
{
    if (bUseQueryResponses != bIsQuery)
    {
        bUseQueryResponses = bIsQuery;
        if (MatrixPanel.IsValid())              { MatrixPanel->SetUseQueryResponses(bIsQuery); }
        if (ActorComparePanel.IsValid())        { ActorComparePanel->SetUseQueryResponses(bIsQuery); }
        if (ComponentInspectorPanel.IsValid())  { ComponentInspectorPanel->SetUseQueryResponses(bIsQuery); }
        if (ExperimentPanel.IsValid())          { ExperimentPanel->SetUseQueryResponses(bIsQuery); }
    }
}

void SCollisionCommanderTab::OnMatrixCellClicked(FName RowPreset, FName ColPreset)
{
    if (ActorComparePanel.IsValid())
    {
        ActorComparePanel->SetupComparison(RowPreset, ColPreset);
    }
    if (ActiveTabIndex != 1)
    {
        ActiveTabIndex = 1;
        Switcher->SetActiveWidgetIndex(1);
    }
}

void SCollisionCommanderTab::RefreshValidation()
{
    if (!ValidationPanel.IsValid()) return;

    const FCCCollisionSnapshot        Snapshot = FCCDataReader::BuildSnapshot();
    const TArray<FCCValidationResult> Results  =
        FCCValidator::RunAll(Snapshot, UCollisionCommanderSettings::GetEnabledRules());

    ValidationPanel->SetResults(Results);

    if (ActorComparePanel.IsValid())
    {
        ActorComparePanel->SetValidationResults(Results);
    }
}

#undef LOCTEXT_NAMESPACE
