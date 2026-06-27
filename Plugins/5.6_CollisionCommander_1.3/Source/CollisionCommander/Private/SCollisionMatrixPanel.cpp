// Copyright 2026 Paracosm. All Rights Reserved.

#include "SCollisionMatrixPanel.h"
#include "CCDataReader.h"
#include "CCPanelStyle.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SButton.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "InputCoreTypes.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "CollisionCommanderModule.h"

#define LOCTEXT_NAMESPACE "CollisionCommander"

// Matrix-specific constants (unique names — safe at file scope)
static constexpr float HeaderW = 144.f;  // row-label cell width
static constexpr float SepPad  =   6.f;  // extra slot padding at builtin/custom boundary

// ----------------------------------------------------------------------------
//  Construct — builds the four-quadrant sticky-header layout
// ----------------------------------------------------------------------------
void SCollisionMatrixPanel::Construct(const FArguments& InArgs)
{
    SAssignNew(HScrollBar, SScrollBar)
        .Orientation(Orient_Horizontal)
        .Thickness(FVector2D(6.f, 6.f));

    SAssignNew(VScrollBar, SScrollBar)
        .Orientation(Orient_Vertical)
        .Thickness(FVector2D(6.f, 6.f));

    // Four content boxes replaced on every RebuildGrid()
    SAssignNew(CornerBox,          SBox);
    SAssignNew(ColHeaderContainer, SBox);
    SAssignNew(RowHeaderContainer, SBox);
    SAssignNew(DataContainer,      SBox);

    // Col-header scroller: horizontal only, no scrollbar UI, never consumes
    // mouse wheel (so wheel always drives vertical scrolling in the data area).
    SAssignNew(ColHeaderScroll, SScrollBox)
        .Orientation(Orient_Horizontal)
        .AllowOverscroll(EAllowOverscroll::No)
        .ConsumeMouseWheel(EConsumeMouseWheel::Never)
        .ScrollBarVisibility(EVisibility::Collapsed)
        + SScrollBox::Slot()[ ColHeaderContainer.ToSharedRef() ];

    // Row-header scroller: vertical only, no scrollbar UI, never consumes wheel.
    SAssignNew(RowHeaderScroll, SScrollBox)
        .Orientation(Orient_Vertical)
        .AllowOverscroll(EAllowOverscroll::No)
        .ConsumeMouseWheel(EConsumeMouseWheel::Never)
        .ScrollBarVisibility(EVisibility::Collapsed)
        + SScrollBox::Slot()[ RowHeaderContainer.ToSharedRef() ];

    // Data scrollers: V-outer drives VScrollBar; H-inner drives HScrollBar.
    SAssignNew(DataHScroll, SScrollBox)
        .Orientation(Orient_Horizontal)
        .ExternalScrollbar(HScrollBar)
        .AllowOverscroll(EAllowOverscroll::No)
        .ConsumeMouseWheel(EConsumeMouseWheel::Never)
        + SScrollBox::Slot()[ DataContainer.ToSharedRef() ];

    // ConsumeMouseWheel::Never: the scroll box does NOT self-scroll and returns
    // FReply::Unhandled() for all wheel events.  This lets them bubble to
    // SCollisionMatrixPanel::OnMouseWheel which handles both normal vertical
    // scroll and Shift+Scroll horizontal from any position in the panel.
    SAssignNew(DataVScroll, SScrollBox)
        .Orientation(Orient_Vertical)
        .ExternalScrollbar(VScrollBar)
        .AllowOverscroll(EAllowOverscroll::No)
        .ConsumeMouseWheel(EConsumeMouseWheel::Never)
        + SScrollBox::Slot()[ DataHScroll.ToSharedRef() ];

    OnCellClicked = InArgs._OnCellClicked;

    ChildSlot
    [
        SNew(SVerticalBox)

        // ── Toolbar ───────────────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(4.f, 4.f, 4.f, 2.f))
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)

            // Update notice — visible only when a newer version is available
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(FMargin(0.f, 0.f, 10.f, 0.f))
            [
                SNew(STextBlock)
                .Text_Lambda([]() -> FText
                {
                    const FString& Ver = FCollisionCommanderModule::GetAvailableVersion();
                    if (Ver.IsEmpty()) return FText::GetEmpty();
                    return FText::Format(
                        LOCTEXT("UpdateNotice",
                            "New version {0} is available now. Reinstall to update."),
                        FText::FromString(Ver));
                })
                .ColorAndOpacity(FCCPanelStyle::TraceChannelHeaderColor)
                .Visibility_Lambda([]() -> EVisibility
                {
                    return FCollisionCommanderModule::GetAvailableVersion().IsEmpty()
                        ? EVisibility::Collapsed : EVisibility::Visible;
                })
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), "FlatButton")
                .OnClicked_Lambda([this]() { Refresh(); return FReply::Handled(); })
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("RefreshBtn", "Refresh"))
                    .Margin(FMargin(6.f, 2.f))
                ]
            ]
        ]

        // ── Matrix — four-quadrant sticky-header layout ───────────────────────
        + SVerticalBox::Slot()
        .FillHeight(1.f)
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(0.f)
            [
                SNew(SVerticalBox)

                // ── Top strip: corner + column headers ─────────────────────────
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SHorizontalBox)

                    // Corner cell (fixed)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        CornerBox.ToSharedRef()
                    ]

                    // Column headers: H-scroll synced to data, no scrollbar UI
                    + SHorizontalBox::Slot()
                    .FillWidth(1.f)
                    [
                        SNew(SBorder)
                        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                        .BorderBackgroundColor(FCCPanelStyle::PanelBgColor)
                        .Padding(FMargin(0.f, 4.f, 4.f, 0.f))
                        [
                            ColHeaderScroll.ToSharedRef()
                        ]
                    ]

                    // Width placeholder that tracks VScrollBar's actual rendered
                    // width every frame, keeping ColHeaderScroll's viewport width
                    // equal to DataHScroll's so scroll offsets stay in sync.
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SBox)
                        .WidthOverride(MakeAttributeLambda([this]() -> FOptionalSize
                        {
                            return VScrollBar.IsValid()
                                ? FOptionalSize(VScrollBar->GetDesiredSize().X)
                                : FOptionalSize(6.f);
                        }))
                    ]
                ]

                // ── Data strip: row headers + data + scroll bars ───────────────
                + SVerticalBox::Slot()
                .FillHeight(1.f)
                [
                    SNew(SHorizontalBox)

                    // Row headers: V-scroll synced to data, no scrollbar UI
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SBorder)
                        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                        .BorderBackgroundColor(FCCPanelStyle::PanelBgColor)
                        .Padding(FMargin(4.f, 0.f, 0.f, 4.f))
                        [
                            // SVerticalBox so the bottom spacer can consume the same
                            // height as HScrollBar in the data area — without this,
                            // RowHeaderScroll's viewport is taller than DataVScroll's
                            // (by exactly HScrollBar.Height), causing the row labels to
                            // drift out of sync with data cells at maximum scroll.
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot()
                            .FillHeight(1.f)
                            [
                                RowHeaderScroll.ToSharedRef()
                            ]
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            [
                                // Spacer whose height tracks HScrollBar's actual rendered
                                // height every frame, keeping both viewport heights equal.
                                SNew(SBox)
                                .HeightOverride(MakeAttributeLambda([this]() -> FOptionalSize
                                {
                                    return HScrollBar.IsValid()
                                        ? FOptionalSize(HScrollBar->GetDesiredSize().Y)
                                        : FOptionalSize();
                                }))
                            ]
                        ]
                    ]

                    // Data area
                    + SHorizontalBox::Slot()
                    .FillWidth(1.f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .FillHeight(1.f)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .FillWidth(1.f)
                            [
                                SNew(SBorder)
                                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                                .BorderBackgroundColor(FCCPanelStyle::PanelBgColor)
                                .Padding(FMargin(0.f, 0.f, 4.f, 4.f))
                                [
                                    DataVScroll.ToSharedRef()
                                ]
                            ]
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            [
                                VScrollBar.ToSharedRef()
                            ]
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            HScrollBar.ToSharedRef()
                        ]
                    ]
                ]
            ]
        ]
    ];

    // Sync col/row header scroll offsets to data every frame.
    RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(
        this, &SCollisionMatrixPanel::SyncScrollPositions));

    // Load plugin icon for the corner cell.  Guarded so packaged/cooked
    // builds that may not have the Resources folder skip gracefully.
    {
        TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("CollisionCommander"));
        if (Plugin.IsValid())
        {
            const FString IconPath = Plugin->GetBaseDir() / TEXT("Resources/Icon128.png");
            if (FPaths::FileExists(IconPath))
            {
                PluginIconBrush = MakeShared<FSlateDynamicImageBrush>(
                    *IconPath, FVector2D(64.f, 64.f));
            }
        }
    }

    Refresh();
}

// ----------------------------------------------------------------------------
//  SyncScrollPositions — called every frame to keep headers aligned with data
// ----------------------------------------------------------------------------
EActiveTimerReturnType SCollisionMatrixPanel::SyncScrollPositions(
    double /*InCurrentTime*/, float /*InDeltaTime*/)
{
    if (ColHeaderScroll.IsValid() && DataHScroll.IsValid())
    {
        ColHeaderScroll->SetScrollOffset(DataHScroll->GetScrollOffset());
    }
    if (RowHeaderScroll.IsValid() && DataVScroll.IsValid())
    {
        RowHeaderScroll->SetScrollOffset(DataVScroll->GetScrollOffset());
    }
    return EActiveTimerReturnType::Continue;
}

// ----------------------------------------------------------------------------
//  Refresh — re-reads from UCollisionProfile and rebuilds the grid
// ----------------------------------------------------------------------------
void SCollisionMatrixPanel::Refresh()
{
    Snapshot = FCCDataReader::BuildSnapshot();
    RebuildGrid();
}

void SCollisionMatrixPanel::SetUseQueryResponses(bool bQuery)
{
    if (bUseQueryResponses != bQuery)
    {
        bUseQueryResponses = bQuery;
        RebuildGrid();
    }
}

// ----------------------------------------------------------------------------
//  RebuildGrid — fills the four content containers from the current Snapshot
//
//  Corner        → CornerBox
//  Col headers   → SHorizontalBox inside ColHeaderContainer
//  Row headers   → SVerticalBox inside RowHeaderContainer
//  Data cells    → SGridPanel inside DataContainer
//
//  Grid line technique: uniform 1px right+bottom slot gap on every cell;
//  the dark PanelBgColor background shows through these gaps.
//  SepPad (6px) is added at the builtin/custom boundary.
// ----------------------------------------------------------------------------
void SCollisionMatrixPanel::RebuildGrid()
{
    const int32 N = Snapshot.PresetCount;
    if (N == 0)
    {
        const TSharedRef<SWidget> Hint =
            SNew(SBox)
            .HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(24.f)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("NoPresets",
                    "No collision presets found.\n"
                    "Open Project Settings \u2192 Collision to add presets."))
                .Justification(ETextJustify::Center)
            ];
        CornerBox->SetContent(SNew(SBox));
        ColHeaderContainer->SetContent(SNew(SBox));
        RowHeaderContainer->SetContent(SNew(SBox));
        DataContainer->SetContent(Hint);
        return;
    }

    // Column-header height from the longest preset name.
    {
        int32 MaxLen = 0;
        for (const FCCPresetData& P : Snapshot.Presets)
            MaxLen = FMath::Max(MaxLen, P.PresetName.ToString().Len());
        DynHeaderH = FMath::Max(static_cast<float>(MaxLen) * 9.5f + 24.f, 80.f);
    }

    constexpr float GL = 1.f;   // grid-line slot gap

    // ── Corner ────────────────────────────────────────────────────────────────
    CornerBox->SetContent(MakeCornerCell());

    // ── Column headers ────────────────────────────────────────────────────────
    const int32 TraceCount = Snapshot.TraceChannels.Num();
    TSharedRef<SHorizontalBox> ColHdrBox = SNew(SHorizontalBox);

    // Trace channel columns first — orange headers, no corresponding rows
    for (int32 Ti = 0; Ti < TraceCount; ++Ti)
    {
        ColHdrBox->AddSlot()
            .AutoWidth()
            .Padding(FMargin(0.f, 0.f, GL, GL))
        [
            MakeTraceColumnHeaderCell(Ti)
        ];
    }

    // Preset columns — first preset column gets SepPad gap after trace section
    for (int32 Idx = 0; Idx < N; ++Idx)
    {
        // Separator at trace/preset boundary or at builtin/custom boundary (not both)
        float LeftPad = 0.f;
        if (TraceCount > 0 && Idx == 0)                       LeftPad = SepPad;
        else if (Idx == Snapshot.BuiltinPresetCount)           LeftPad = SepPad;

        ColHdrBox->AddSlot()
            .AutoWidth()
            .Padding(FMargin(LeftPad, 0.f, GL, GL))
        [
            MakeColumnHeaderCell(Idx)
        ];
    }
    ColHeaderContainer->SetContent(ColHdrBox);

    // ── Row headers ───────────────────────────────────────────────────────────
    TSharedRef<SVerticalBox> RowHdrBox = SNew(SVerticalBox);
    for (int32 Idx = 0; Idx < N; ++Idx)
    {
        const bool bSep = (Idx == Snapshot.BuiltinPresetCount);
        RowHdrBox->AddSlot()
            .AutoHeight()
            .Padding(FMargin(0.f, bSep ? SepPad : 0.f, GL, GL))
        [
            MakeRowHeaderCell(Idx)
        ];
    }
    RowHeaderContainer->SetContent(RowHdrBox);

    // ── Data grid ─────────────────────────────────────────────────────────────
    // Columns 0..TraceCount-1: trace channel cells (direct response read, no pairwise)
    // Columns TraceCount..TraceCount+N-1: preset vs preset cells (existing pairwise logic)
    TSharedRef<SGridPanel> DataGrid = SNew(SGridPanel);
    for (int32 A = 0; A < N; ++A)
    {
        const bool bSepRow = (A == Snapshot.BuiltinPresetCount);

        // Trace channel columns for this row
        for (int32 Ti = 0; Ti < TraceCount; ++Ti)
        {
            DataGrid->AddSlot(Ti, A)
                .Padding(FMargin(0.f, bSepRow ? SepPad : 0.f, GL, GL))
            [
                MakeTraceDataCell(A, Ti)
            ];
        }

        // Preset columns — shifted right by TraceCount
        for (int32 B = 0; B < N; ++B)
        {
            float LeftPad = 0.f;
            if (TraceCount > 0 && B == 0)                 LeftPad = SepPad;
            else if (B == Snapshot.BuiltinPresetCount)     LeftPad = SepPad;

            DataGrid->AddSlot(TraceCount + B, A)
                .Padding(FMargin(LeftPad, bSepRow ? SepPad : 0.f, GL, GL))
            [
                MakeDataCell(A, B)
            ];
        }
    }
    DataContainer->SetContent(DataGrid);
}

// ----------------------------------------------------------------------------
//  Cell factories
// ----------------------------------------------------------------------------
TSharedRef<SWidget> SCollisionMatrixPanel::MakeCornerCell() const
{
    // Outer SBorder: PanelBgColor with 4px right + 4px bottom padding.
    // This mirrors the col-header section's 4px top strip and the row-header
    // section's 4px left strip, so the corner box has matching accent lines
    // on all edges that face the adjacent header sections.
    // Total outer size = (HeaderW + 4.f) wide × same height as col headers — unchanged.

    // Inner content: SOverlay so the plugin icon can sit centred on top of
    // the header background without disturbing the fixed size SBox underneath.
    TSharedRef<SOverlay> Inner = SNew(SOverlay)

        // Layer 0: dark background at fixed size (unchanged from before)
        + SOverlay::Slot()
        [
            SNew(SBox)
            .WidthOverride(HeaderW)
            .HeightOverride(200.f)
        ]

        // Layer 1: plugin icon — only added when the brush was loaded
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SImage)
            .Image(PluginIconBrush.IsValid() ? PluginIconBrush.Get() : nullptr)
            .ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, PluginIconBrush.IsValid() ? 0.55f : 0.f))
            .Visibility(PluginIconBrush.IsValid() ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
        ];

    return SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FCCPanelStyle::PanelBgColor)
        .Padding(FMargin(0.f, 0.f, 4.f, 4.f))
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FCCPanelStyle::HeaderBgColor)
            .Padding(0.f)
            [
                Inner
            ]
        ];
}

TSharedRef<SWidget> SCollisionMatrixPanel::MakeColumnHeaderCell(int32 PresetIndex) const
{
    const FCCPresetData& Preset = Snapshot.Presets[PresetIndex];
    const FName& Name           = Preset.PresetName;

    // Tooltip: preset name + object type channel
    const FText Tooltip = FText::Format(
        LOCTEXT("ColHdrTooltip", "{0}\nObject type: {1}"),
        FText::FromName(Name),
        FText::FromName(Preset.ObjectTypeChannel));

    return SNew(SBox)
        .WidthOverride(FCCPanelStyle::CellW)
        .HeightOverride(200.f)
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FCCPanelStyle::HeaderBgColor)
            .Padding(0.f)
            .ToolTipText(Tooltip)
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SNew(SBox)
                    .WidthOverride(190.f)
                    .HeightOverride(FCCPanelStyle::CellW)
                    .VAlign(VAlign_Center)
                    .RenderTransform(FSlateRenderTransform(
                        FQuat2D(FMath::DegreesToRadians(-90.f))))
                    .RenderTransformPivot(FVector2D(0.5f, 0.5f))
                    [
                        SNew(STextBlock)
                        .Text(FText::FromName(Name))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
                        .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                    ]
                ]
            ]
        ];
}

TSharedRef<SWidget> SCollisionMatrixPanel::MakeRowHeaderCell(int32 PresetIndex) const
{
    const FCCPresetData& Preset = Snapshot.Presets[PresetIndex];
    const FName& Name           = Preset.PresetName;

    // Tooltip: preset name + object type channel
    const FText Tooltip = FText::Format(
        LOCTEXT("RowHdrTooltip", "{0}\nObject type: {1}"),
        FText::FromName(Name),
        FText::FromName(Preset.ObjectTypeChannel));

    return SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FCCPanelStyle::HeaderBgColor)
        .Padding(FMargin(6.f, FCCPanelStyle::CellPad))
        .ToolTipText(Tooltip)
        [
            SNew(SBox)
            .WidthOverride(HeaderW - 12.f)
            .HeightOverride(FCCPanelStyle::CellH - 2.f * FCCPanelStyle::CellPad)
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(FText::FromName(Name))
                .Justification(ETextJustify::Left)
            ]
        ];
}

TSharedRef<SWidget> SCollisionMatrixPanel::MakeDataCell(int32 A, int32 B) const
{
    // Shared rounded brush — white fill tinted via BorderBackgroundColor.
    // static const: constructed once, valid for the plugin lifetime.
    static const FSlateRoundedBoxBrush RoundedBrush(FLinearColor::White, 3.f);

    const bool bIsDiag = (A == B);

    // ── Diagonal cell (preset vs itself) — dark blue, hover highlight ─────────
    if (bIsDiag)
    {
        const FCCPairwiseResult& DiagResult =
            Snapshot.PairwiseMatrix[A * Snapshot.PresetCount + A];
        const ECollisionResponse DiagResp =
            bUseQueryResponses ? DiagResult.QueryResult : DiagResult.PhysicsResult;

        TSharedPtr<bool> bHovered = MakeShared<bool>(false);

        // Same outer-border hover outline + inner fill brightening used by
        // non-diagonal cells so all hover animations are visually consistent.
        TSharedRef<SBorder> DiagRoot = SNew(SBorder)
            .BorderImage(&RoundedBrush)
            .BorderBackgroundColor_Lambda([bHovered]() -> FSlateColor
            {
                return *bHovered
                    ? FLinearColor(1.f, 1.f, 1.f, 0.70f)
                    : FLinearColor(0.f, 0.f, 0.f, 0.f);
            })
            .Padding(1.f)
            .ToolTipText(BuildCellTooltip(Snapshot, A, A, bUseQueryResponses))
            [
                SNew(SBorder)
                .BorderImage(&RoundedBrush)
                .BorderBackgroundColor_Lambda([bHovered]() -> FSlateColor
                {
                    const FLinearColor Diag(0.04f, 0.04f, 0.30f, 1.f);
                    if (*bHovered)
                    {
                        return FLinearColor(
                            FMath::Min(Diag.R * 1.45f, 1.f),
                            FMath::Min(Diag.G * 1.45f, 1.f),
                            FMath::Min(Diag.B * 1.45f, 1.f), 1.f);
                    }
                    return Diag;
                })
                .Padding(FCCPanelStyle::CellPad)
                [
                    SNew(SBox)
                    .WidthOverride(FCCPanelStyle::CellW - 2.f * FCCPanelStyle::CellPad - 2.f)
                    .HeightOverride(FCCPanelStyle::CellH - 2.f * FCCPanelStyle::CellPad - 2.f)
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(ResponseToString(DiagResp)))
                        .ColorAndOpacity_Lambda([bHovered]() -> FSlateColor
                        {
                            return *bHovered
                                ? FLinearColor(1.f, 1.f, 1.f, 1.f)
                                : FLinearColor(1.f, 1.f, 1.f, 0.6f);
                        })
                        .Justification(ETextJustify::Center)
                    ]
                ]
            ];

        DiagRoot->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda(
            [bHovered](const FGeometry&, const FPointerEvent&) { *bHovered = true; }));
        DiagRoot->SetOnMouseLeave(FSimpleNoReplyPointerEventHandler::CreateLambda(
            [bHovered](const FPointerEvent&) { *bHovered = false; }));

        return DiagRoot;
    }

    // ── Data cell — colour by resolved response, hover animation, click ───────
    const FCCPairwiseResult& PairResult =
        Snapshot.PairwiseMatrix[A * Snapshot.PresetCount + B];
    const ECollisionResponse Resp =
        bUseQueryResponses ? PairResult.QueryResult : PairResult.PhysicsResult;

    FLinearColor BgColor;
    switch (Resp)
    {
        case ECR_Block:   BgColor = FCCPanelStyle::BlockColor;   break;
        case ECR_Overlap: BgColor = FCCPanelStyle::OverlapColor; break;
        default:          BgColor = FCCPanelStyle::IgnoreColor;  break;
    }

    const FName RowPresetName = Snapshot.Presets[A].PresetName;
    const FName ColPresetName = Snapshot.Presets[B].PresetName;

    // Hover state shared by the outer outline border and inner fill + text.
    TSharedPtr<bool> bHovered = MakeShared<bool>(false);

    TSharedRef<SBorder> CellRoot = SNew(SBorder)
        .BorderImage(&RoundedBrush)
        .BorderBackgroundColor_Lambda([bHovered]() -> FSlateColor
        {
            return *bHovered
                ? FLinearColor(1.f, 1.f, 1.f, 0.70f)
                : FLinearColor(0.f, 0.f, 0.f, 0.f);
        })
        .Padding(1.f)
        .Cursor(EMouseCursor::Hand)
        .ToolTipText(BuildCellTooltip(Snapshot, A, B, bUseQueryResponses))
        .OnMouseButtonDown_Lambda([this, RowPresetName, ColPresetName](
            const FGeometry&, const FPointerEvent& Event) -> FReply
        {
            if (Event.GetEffectingButton() == EKeys::LeftMouseButton)
            {
                OnCellClicked.ExecuteIfBound(RowPresetName, ColPresetName);
                return FReply::Handled();
            }
            return FReply::Unhandled();
        })
        [
            SNew(SBorder)
            .BorderImage(&RoundedBrush)
            .BorderBackgroundColor_Lambda([BgColor, bHovered]() -> FSlateColor
            {
                if (*bHovered)
                {
                    return FLinearColor(
                        FMath::Min(BgColor.R * 1.45f, 1.f),
                        FMath::Min(BgColor.G * 1.45f, 1.f),
                        FMath::Min(BgColor.B * 1.45f, 1.f), 1.f);
                }
                return BgColor;
            })
            .Padding(FCCPanelStyle::CellPad)
            [
                SNew(SBox)
                .WidthOverride(FCCPanelStyle::CellW - 2.f * FCCPanelStyle::CellPad - 2.f)
                .HeightOverride(FCCPanelStyle::CellH - 2.f * FCCPanelStyle::CellPad - 2.f)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(ResponseToString(Resp)))
                    .ColorAndOpacity_Lambda([bHovered]() -> FSlateColor
                    {
                        return *bHovered
                            ? FLinearColor(1.f, 1.f, 1.f, 1.f)
                            : FLinearColor(1.f, 1.f, 1.f, 0.85f);
                    })
                    .Justification(ETextJustify::Center)
                ]
            ]
        ];

    CellRoot->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda(
        [bHovered](const FGeometry&, const FPointerEvent&) { *bHovered = true; }));
    CellRoot->SetOnMouseLeave(FSimpleNoReplyPointerEventHandler::CreateLambda(
        [bHovered](const FPointerEvent&) { *bHovered = false; }));

    return CellRoot;
}

// ----------------------------------------------------------------------------
//  MakeTraceColumnHeaderCell — orange column header for a trace channel
// ----------------------------------------------------------------------------
TSharedRef<SWidget> SCollisionMatrixPanel::MakeTraceColumnHeaderCell(int32 TraceIdx) const
{
    const FCCChannelInfo& TraceChannel = Snapshot.TraceChannels[TraceIdx];
    const FString DisplayName = FCCPanelStyle::FormatChannelDisplayName(TraceChannel.Name);

    return SNew(SBox)
        .WidthOverride(FCCPanelStyle::CellW)
        .HeightOverride(200.f)
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FCCPanelStyle::TraceChannelHeaderColor)
            .Padding(0.f)
            .ToolTipText(FText::FromName(TraceChannel.Name))
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SNew(SBox)
                    .WidthOverride(190.f)
                    .HeightOverride(FCCPanelStyle::CellW)
                    .VAlign(VAlign_Center)
                    .RenderTransform(FSlateRenderTransform(
                        FQuat2D(FMath::DegreesToRadians(-90.f))))
                    .RenderTransformPivot(FVector2D(0.5f, 0.5f))
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(DisplayName))
                        .ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.92f))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
                        .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                    ]
                ]
            ]
        ];
}

// ----------------------------------------------------------------------------
//  MakeTraceDataCell — direct response read for a preset row vs trace channel
//
//  Trace channels are one-directional: there is no "B's response to A's type"
//  because trace channels carry no ObjectType. The cell shows the preset's
//  Query response to the trace channel, regardless of the Query/Physics toggle
//  (trace channels are query-only in UE; Physics response is not meaningful).
//  Hover animation matches existing data cells. Click is a no-op.
// ----------------------------------------------------------------------------
TSharedRef<SWidget> SCollisionMatrixPanel::MakeTraceDataCell(int32 PresetRow, int32 TraceIdx) const
{
    static const FSlateRoundedBoxBrush RoundedBrush(FLinearColor::White, 3.f);

    const FCCPresetData& Preset      = Snapshot.Presets[PresetRow];
    const FCCChannelInfo& TraceChannel = Snapshot.TraceChannels[TraceIdx];

    const ECollisionResponse* Found = Preset.QueryResponses.Find(TraceChannel.Name);
    const ECollisionResponse  Resp  = Found ? *Found : ECR_Ignore;

    FLinearColor BgColor;
    switch (Resp)
    {
        case ECR_Block:   BgColor = FCCPanelStyle::BlockColor;   break;
        case ECR_Overlap: BgColor = FCCPanelStyle::OverlapColor; break;
        default:          BgColor = FCCPanelStyle::IgnoreColor;  break;
    }

    TSharedPtr<bool> bHovered = MakeShared<bool>(false);

    TSharedRef<SBorder> CellRoot = SNew(SBorder)
        .BorderImage(&RoundedBrush)
        .BorderBackgroundColor_Lambda([bHovered]() -> FSlateColor
        {
            return *bHovered
                ? FLinearColor(1.f, 1.f, 1.f, 0.70f)
                : FLinearColor(0.f, 0.f, 0.f, 0.f);
        })
        .Padding(1.f)
        .ToolTipText(BuildTraceDataCellTooltip(Snapshot, PresetRow, TraceIdx))
        [
            SNew(SBorder)
            .BorderImage(&RoundedBrush)
            .BorderBackgroundColor_Lambda([BgColor, bHovered]() -> FSlateColor
            {
                if (*bHovered)
                {
                    return FLinearColor(
                        FMath::Min(BgColor.R * 1.45f, 1.f),
                        FMath::Min(BgColor.G * 1.45f, 1.f),
                        FMath::Min(BgColor.B * 1.45f, 1.f), 1.f);
                }
                return BgColor;
            })
            .Padding(FCCPanelStyle::CellPad)
            [
                SNew(SBox)
                .WidthOverride(FCCPanelStyle::CellW - 2.f * FCCPanelStyle::CellPad - 2.f)
                .HeightOverride(FCCPanelStyle::CellH - 2.f * FCCPanelStyle::CellPad - 2.f)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(ResponseToString(Resp)))
                    .ColorAndOpacity_Lambda([bHovered]() -> FSlateColor
                    {
                        return *bHovered
                            ? FLinearColor(1.f, 1.f, 1.f, 1.f)
                            : FLinearColor(1.f, 1.f, 1.f, 0.85f);
                    })
                    .Justification(ETextJustify::Center)
                ]
            ]
        ];

    CellRoot->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda(
        [bHovered](const FGeometry&, const FPointerEvent&) { *bHovered = true; }));
    CellRoot->SetOnMouseLeave(FSimpleNoReplyPointerEventHandler::CreateLambda(
        [bHovered](const FPointerEvent&) { *bHovered = false; }));

    return CellRoot;
}

// ----------------------------------------------------------------------------
//  Tooltip text builder
//
//  Format:
//    <PresetA>  →  <PresetB>:  <A's response to B's object type>
//    <PresetB>  →  <PresetA>:  <B's response to A's object type>
//
//    Resolved:  <result>
// ----------------------------------------------------------------------------
FText SCollisionMatrixPanel::BuildCellTooltip(
    const FCCCollisionSnapshot& Snap, int32 A, int32 B, bool bQuery)
{
    const FCCPresetData& PA = Snap.Presets[A];
    const FCCPresetData& PB = Snap.Presets[B];

    const TMap<FName, ECollisionResponse>& AMap = bQuery ? PA.QueryResponses : PA.PhysicsResponses;
    const TMap<FName, ECollisionResponse>& BMap = bQuery ? PB.QueryResponses : PB.PhysicsResponses;

    const ECollisionResponse* AtoBPtr = AMap.Find(PB.ObjectTypeChannel);
    const ECollisionResponse* BtoAPtr = BMap.Find(PA.ObjectTypeChannel);

    const FString AtoB = AtoBPtr ? ResponseToString(*AtoBPtr) : TEXT("?");
    const FString BtoA = BtoAPtr ? ResponseToString(*BtoAPtr) : TEXT("?");

    const FCCPairwiseResult& Pair = Snap.PairwiseMatrix[A * Snap.PresetCount + B];
    const FString Resolved = ResponseToString(bQuery ? Pair.QueryResult : Pair.PhysicsResult);

    return FText::Format(
        LOCTEXT("CellTooltipFmt",
            "{0}  \u2192  {1}:  {2}\n{1}  \u2192  {0}:  {3}\n\nResolved:  {4}"),
        FText::FromName(PA.PresetName),
        FText::FromName(PB.PresetName),
        FText::FromString(AtoB),
        FText::FromString(BtoA),
        FText::FromString(Resolved));
}

// ----------------------------------------------------------------------------
//  BuildTraceDataCellTooltip — single-direction tooltip for trace cells
//
//  Format:
//    <PresetName>  →  <TraceChannelName>:  Block/Overlap/Ignore
//    (Trace channels are query-only — no pairwise direction exists)
// ----------------------------------------------------------------------------
FText SCollisionMatrixPanel::BuildTraceDataCellTooltip(
    const FCCCollisionSnapshot& Snap, int32 PresetRow, int32 TraceIdx)
{
    const FCCPresetData&  Preset  = Snap.Presets[PresetRow];
    const FCCChannelInfo& Channel = Snap.TraceChannels[TraceIdx];

    const ECollisionResponse* Found = Preset.QueryResponses.Find(Channel.Name);
    const FString Response = Found ? ResponseToString(*Found) : TEXT("?");
    const FString DisplayName = FCCPanelStyle::FormatChannelDisplayName(Channel.Name);

    return FText::Format(
        LOCTEXT("TraceDataCellTooltipFmt", "{0}  \u2192  {1}:  {2}"),
        FText::FromName(Preset.PresetName),
        FText::FromString(DisplayName),
        FText::FromString(Response));
}

FString SCollisionMatrixPanel::ResponseToString(ECollisionResponse R)
{
    switch (R)
    {
        case ECR_Block:   return TEXT("Block");
        case ECR_Overlap: return TEXT("Overlap");
        default:          return TEXT("Ignore");
    }
}

// ----------------------------------------------------------------------------
//  Mouse wheel — panel-level handler
//
//  DataVScroll uses ConsumeMouseWheel::Never so wheel events always bubble
//  here from data cells, column headers, and row headers alike.
//  Shift held  → horizontal scroll via DataHScroll.
//  No modifier → vertical scroll via DataVScroll.
// ----------------------------------------------------------------------------
FReply SCollisionMatrixPanel::OnMouseWheel(
    const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    const float Delta = MouseEvent.GetWheelDelta();
    if (MouseEvent.IsShiftDown())
    {
        if (DataHScroll.IsValid())
        {
            DataHScroll->SetScrollOffset(DataHScroll->GetScrollOffset() - Delta * 40.f);
            return FReply::Handled();
        }
    }
    else
    {
        if (DataVScroll.IsValid())
        {
            DataVScroll->SetScrollOffset(DataVScroll->GetScrollOffset() - Delta * 40.f);
            return FReply::Handled();
        }
    }
    return SCompoundWidget::OnMouseWheel(MyGeometry, MouseEvent);
}

#undef LOCTEXT_NAMESPACE
