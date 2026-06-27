// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "CCTypes.h"

class SBox;
class STableViewBase;
class ITableRow;

DECLARE_DELEGATE_OneParam(FCCNavigateToTabDelegate, int32 /*TabIndex*/);

// ============================================================================
//  SCollisionValidationPanel
//
//  Validation tab panel (tab index 4).  Runs FCCValidator::RunAll() against
//  the live UCollisionProfile and displays results grouped by severity:
//    Error   (red)    — misconfiguration that will silently break collision
//    Warning (yellow) — likely unintended setup, worth investigating
//    Info    (grey)   — FYI observations, not necessarily wrong
//
//  Each severity group is a collapsible SExpandableArea containing an
//  SListView of result rows.  Each row shows the rule ID, description, and
//  suggested fix.  Hovering shows a full-text tooltip (description + fix +
//  affected preset/channel names).  Double-clicking any row navigates to the
//  Matrix tab (index 0) via the OnNavigateToTab delegate.
//
//  Results are pushed via SetResults() (called by SCollisionCommanderTab after
//  each refresh/commit). The "Run Validation" button fires OnRunValidationRequested
//  so the tab runs the pass and keeps all panels in sync.
// ============================================================================
class SCollisionValidationPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCollisionValidationPanel) {}
        SLATE_EVENT(FCCNavigateToTabDelegate, OnNavigateToTab)
        // Fired when the user clicks "Run Validation" inside the panel.
        // The tab wires this to RefreshValidation() so all panels stay in sync.
        SLATE_EVENT(FSimpleDelegate, OnRunValidationRequested)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // Replace the current result set and rebuild the display.
    // Called externally by SCollisionCommanderTab::RefreshValidation().
    void SetResults(const TArray<FCCValidationResult>& Results);

    // Replace the component-level result set and rebuild.
    // Called by SCollisionCommanderTab when the Inspector refreshes.
    void SetComponentResults(const TArray<FCCValidationResult>& Results);

    // Read-only accessors used by the tab strip summary badge.
    bool  HasResults() const;
    int32 GetResultCount(ECCValidationSeverity Sev) const;

private:
    // ── Data ────────────────────────────────────────────────────────────────
    TArray<FCCValidationResult>             CachedResults;           // from FCCValidator::RunAll
    TArray<FCCValidationResult>             CachedComponentResults;  // from FCCValidator::RunComponentChecks

    // Per-severity item arrays whose addresses are passed to SListView as
    // ListItemsSource.  They are member arrays so the raw pointer remains
    // valid for the lifetime of the panel.
    TArray<TSharedPtr<FCCValidationResult>> ErrorItems;
    TArray<TSharedPtr<FCCValidationResult>> WarningItems;
    TArray<TSharedPtr<FCCValidationResult>> InfoItems;

    FCCNavigateToTabDelegate OnNavigateToTab;
    FSimpleDelegate          OnRunValidationRequested;

    // Container replaced entirely on every Rebuild().
    TSharedPtr<SBox> ContentBox;

    // ── Internal ────────────────────────────────────────────────────────────
    void   Rebuild();
    FReply OnRunNowClicked();

    // Builds a collapsible SExpandableArea with an SListView for one severity.
    // Items must be a member array (stable address for SListView pointer).
    TSharedRef<SWidget> MakeSeveritySection(
        ECCValidationSeverity                    Sev,
        TArray<TSharedPtr<FCCValidationResult>>& Items);

    // SListView row factory.
    TSharedRef<ITableRow> OnGenerateRow(
        TSharedPtr<FCCValidationResult>   Item,
        const TSharedRef<STableViewBase>& OwnerTable);

    // Navigate to Matrix tab on double-click.
    void OnResultDoubleClicked(TSharedPtr<FCCValidationResult> Item);

    // ── Statics ─────────────────────────────────────────────────────────────
    static FLinearColor SeverityColor(ECCValidationSeverity Sev);
    static FText        SeverityLabel(ECCValidationSeverity Sev);
};
