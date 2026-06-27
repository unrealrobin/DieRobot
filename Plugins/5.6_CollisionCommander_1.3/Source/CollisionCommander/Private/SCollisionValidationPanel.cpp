// Copyright 2026 Paracosm. All Rights Reserved.

#include "SCollisionValidationPanel.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "CollisionCommander"

// Height of each result row in the list views.
static constexpr float GValRowHeight = 28.f;


// ---------------------------------------------------------------------------
//  Statics
// ---------------------------------------------------------------------------

FLinearColor SCollisionValidationPanel::SeverityColor(ECCValidationSeverity Sev)
{
    switch (Sev)
    {
    case ECCValidationSeverity::Error:   return FLinearColor(0.80f, 0.10f, 0.10f);
    case ECCValidationSeverity::Warning: return FLinearColor(0.75f, 0.55f, 0.00f);
    case ECCValidationSeverity::Info:    return FLinearColor(0.45f, 0.45f, 0.45f);
    }
    return FLinearColor::White;
}

FText SCollisionValidationPanel::SeverityLabel(ECCValidationSeverity Sev)
{
    switch (Sev)
    {
    case ECCValidationSeverity::Error:   return LOCTEXT("SevError",   "ERRORS");
    case ECCValidationSeverity::Warning: return LOCTEXT("SevWarning", "WARNINGS");
    case ECCValidationSeverity::Info:    return LOCTEXT("SevInfo",    "INFO");
    }
    return FText::GetEmpty();
}


// ---------------------------------------------------------------------------
//  Construct
// ---------------------------------------------------------------------------

void SCollisionValidationPanel::Construct(const FArguments& InArgs)
{
    OnNavigateToTab          = InArgs._OnNavigateToTab;
    OnRunValidationRequested = InArgs._OnRunValidationRequested;

    ChildSlot
    [
        SNew(SVerticalBox)

        // ── Toolbar row ──────────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(FMargin(6.f, 4.f))
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(LOCTEXT("RunNowBtn", "Run Validation"))
                    .ToolTipText(LOCTEXT("RunNowTip",
                        "Re-read the live collision configuration and run all enabled rules."))
                    .OnClicked(this, &SCollisionValidationPanel::OnRunNowClicked)
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(10.f, 0.f, 0.f, 0.f))
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("DoubleClickHint",
                        "Double-click any result to navigate to the Matrix tab."))
                    .ColorAndOpacity(FLinearColor(0.55f, 0.55f, 0.55f))
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
                ]
            ]
        ]

        // ── Results scroll area ──────────────────────────────────────────────
        + SVerticalBox::Slot()
        .FillHeight(1.f)
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()
            .Padding(FMargin(4.f))
            [
                SAssignNew(ContentBox, SBox)
            ]
        ]
    ];

    // Start with the empty-state view.
    Rebuild();
}


// ---------------------------------------------------------------------------
//  SetResults / OnRunNowClicked / Rebuild
// ---------------------------------------------------------------------------

void SCollisionValidationPanel::SetResults(const TArray<FCCValidationResult>& Results)
{
    CachedResults = Results;
    Rebuild();
}

void SCollisionValidationPanel::SetComponentResults(const TArray<FCCValidationResult>& Results)
{
    CachedComponentResults = Results;
    Rebuild();
}

bool SCollisionValidationPanel::HasResults() const
{
    return !CachedResults.IsEmpty() || !CachedComponentResults.IsEmpty();
}

int32 SCollisionValidationPanel::GetResultCount(ECCValidationSeverity Sev) const
{
    int32 Count = 0;
    for (const FCCValidationResult& R : CachedResults)
    {
        if (R.Severity == Sev) { ++Count; }
    }
    for (const FCCValidationResult& R : CachedComponentResults)
    {
        if (R.Severity == Sev) { ++Count; }
    }
    return Count;
}

FReply SCollisionValidationPanel::OnRunNowClicked()
{
    // Delegate to the tab so RefreshValidation() updates all panels (including
    // Actor Compare's overlap warning icons) in a single coordinated pass.
    OnRunValidationRequested.ExecuteIfBound();
    return FReply::Handled();
}

void SCollisionValidationPanel::Rebuild()
{
    if (!ContentBox.IsValid()) return;

    // Repopulate per-severity item arrays — preset/channel results first,
    // then component results appended to the same buckets.
    ErrorItems.Reset();
    WarningItems.Reset();
    InfoItems.Reset();

    auto AddToItems = [&](const FCCValidationResult& R)
    {
        TSharedPtr<FCCValidationResult> Item = MakeShared<FCCValidationResult>(R);
        switch (R.Severity)
        {
        case ECCValidationSeverity::Error:   ErrorItems.Add(Item);   break;
        case ECCValidationSeverity::Warning: WarningItems.Add(Item); break;
        case ECCValidationSeverity::Info:    InfoItems.Add(Item);    break;
        }
    };

    for (const FCCValidationResult& R : CachedResults)          { AddToItems(R); }
    for (const FCCValidationResult& R : CachedComponentResults) { AddToItems(R); }

    // ── Empty state ──────────────────────────────────────────────────────────
    if (ErrorItems.IsEmpty() && WarningItems.IsEmpty() && InfoItems.IsEmpty())
    {
        ContentBox->SetContent(
            SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            .Padding(FMargin(0.f, 48.f))
            [
                SNew(STextBlock)
                .Text(LOCTEXT("NoIssues", "\u2713  No issues found"))
                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
                .ColorAndOpacity(FLinearColor(0.35f, 0.75f, 0.35f))
            ]);
        return;
    }

    // ── Severity sections ─────────────────────────────────────────────────────
    TSharedRef<SVerticalBox> VBox = SNew(SVerticalBox);

    if (!ErrorItems.IsEmpty())
    {
        VBox->AddSlot()
        .AutoHeight()
        .Padding(FMargin(0.f, 0.f, 0.f, 6.f))
        [
            MakeSeveritySection(ECCValidationSeverity::Error, ErrorItems)
        ];
    }
    if (!WarningItems.IsEmpty())
    {
        VBox->AddSlot()
        .AutoHeight()
        .Padding(FMargin(0.f, 0.f, 0.f, 6.f))
        [
            MakeSeveritySection(ECCValidationSeverity::Warning, WarningItems)
        ];
    }
    if (!InfoItems.IsEmpty())
    {
        VBox->AddSlot()
        .AutoHeight()
        [
            MakeSeveritySection(ECCValidationSeverity::Info, InfoItems)
        ];
    }

    ContentBox->SetContent(VBox);
}


// ---------------------------------------------------------------------------
//  MakeSeveritySection
// ---------------------------------------------------------------------------

TSharedRef<SWidget> SCollisionValidationPanel::MakeSeveritySection(
    ECCValidationSeverity                    Sev,
    TArray<TSharedPtr<FCCValidationResult>>& Items)
{
    const FLinearColor SevColor = SeverityColor(Sev);
    const int32        Count    = Items.Num();

    return SNew(SExpandableArea)
    .InitiallyCollapsed(false)
    .Padding(FMargin(1.f))
    .HeaderContent()
    [
        // Custom header: coloured stripe + bold label.
        SNew(SHorizontalBox)

        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Fill)
        [
            SNew(SBox)
            .WidthOverride(4.f)
            [
                SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush("WhiteBrush"))
                .BorderBackgroundColor(SevColor)
            ]
        ]

        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(8.f, 4.f))
        [
            SNew(STextBlock)
            .Text(FText::Format(
                LOCTEXT("SevHeaderFmt", "{0}  ({1})"),
                SeverityLabel(Sev),
                FText::AsNumber(Count)))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
            .ColorAndOpacity(SevColor)
        ]
    ]
    .BodyContent()
    [
        // Fixed-height box so SListView does not try to scroll internally.
        // The outer SScrollBox owns all scrolling.
        SNew(SBox)
        .HeightOverride(GValRowHeight * static_cast<float>(Count))
        [
            SNew(SListView<TSharedPtr<FCCValidationResult>>)
            .ListItemsSource(&Items)
            .OnGenerateRow(this, &SCollisionValidationPanel::OnGenerateRow)
            .OnMouseButtonDoubleClick(this, &SCollisionValidationPanel::OnResultDoubleClicked)
            .SelectionMode(ESelectionMode::Single)
            .ScrollbarVisibility(EVisibility::Collapsed)
            .ConsumeMouseWheel(EConsumeMouseWheel::Never)
        ]
    ];
}


// ---------------------------------------------------------------------------
//  OnGenerateRow
// ---------------------------------------------------------------------------

TSharedRef<ITableRow> SCollisionValidationPanel::OnGenerateRow(
    TSharedPtr<FCCValidationResult>   Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    const FLinearColor SevColor = SeverityColor(Item->Severity);

    // Build full tooltip text (description + fix + affected names).
    FString Tooltip = Item->Description;

    if (!Item->SuggestedFix.IsEmpty())
    {
        Tooltip += TEXT("\n\nFix: ");
        Tooltip += Item->SuggestedFix;
    }
    if (!Item->AffectedPresets.IsEmpty())
    {
        Tooltip += TEXT("\n\nAffected presets: ");
        for (int32 i = 0; i < Item->AffectedPresets.Num(); ++i)
        {
            if (i > 0) Tooltip += TEXT(", ");
            Tooltip += Item->AffectedPresets[i].ToString();
        }
    }
    if (!Item->AffectedChannels.IsEmpty())
    {
        Tooltip += TEXT("\n\nAffected channels: ");
        for (int32 i = 0; i < Item->AffectedChannels.Num(); ++i)
        {
            if (i > 0) Tooltip += TEXT(", ");
            Tooltip += Item->AffectedChannels[i].ToString();
        }
    }

    return SNew(STableRow<TSharedPtr<FCCValidationResult>>, OwnerTable)
    .ToolTipText(FText::FromString(Tooltip))
    [
        SNew(SHorizontalBox)

        // Left severity stripe — 4 px wide, full row height.
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("WhiteBrush"))
            .BorderBackgroundColor(SevColor)
            .Padding(FMargin(3.f, 0.f, 0.f, 0.f))
        ]

        // Rule ID — fixed width, monospace, severity-coloured.
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(8.f, 0.f, 12.f, 0.f))
        [
            SNew(SBox)
            .WidthOverride(145.f)
            [
                SNew(STextBlock)
                .Text(FText::FromName(Item->RuleId))
                .Font(FCoreStyle::GetDefaultFontStyle("Mono", 8))
                .ColorAndOpacity(SevColor)
            ]
        ]

        // Description — fills the remaining horizontal space.
        + SHorizontalBox::Slot()
        .FillWidth(1.f)
        .VAlign(VAlign_Center)
        .Padding(FMargin(0.f, 0.f, 8.f, 0.f))
        [
            SNew(STextBlock)
            .Text(FText::FromString(Item->Description))
            .ColorAndOpacity(FLinearColor(0.88f, 0.88f, 0.88f))
        ]

        // Suggested fix — fixed width, italic, dimmed.
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(0.f, 0.f, 8.f, 0.f))
        [
            SNew(SBox)
            .WidthOverride(210.f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Item->SuggestedFix))
                .Font(FCoreStyle::GetDefaultFontStyle("Italic", 8))
                .ColorAndOpacity(FLinearColor(0.50f, 0.50f, 0.50f))
            ]
        ]
    ];
}


// ---------------------------------------------------------------------------
//  OnResultDoubleClicked
// ---------------------------------------------------------------------------

void SCollisionValidationPanel::OnResultDoubleClicked(TSharedPtr<FCCValidationResult> /*Item*/)
{
    // Navigate to the Matrix tab (index 0).
    OnNavigateToTab.ExecuteIfBound(0);
}

#undef LOCTEXT_NAMESPACE
