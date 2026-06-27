// Copyright 2026 Paracosm. All Rights Reserved.

#include "SCollisionExperimentPanel.h"
#include "CCDataReader.h"
#include "CCPanelStyle.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableText.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"

#define LOCTEXT_NAMESPACE "CollisionCommander"

// Experiment-specific constants (unique names — safe at file scope)
// Sentinel placed at the top of the source preset combo.
// When selected, it initialises a blank preset (all Ignore, empty name).
static const FName NewPresetSentinel(TEXT("__CC_NEW_PRESET__"));

// Channel response combo label colours — intentionally brighter than the
// standard matrix colours for legibility inside a small combo widget.
static const FLinearColor ResponseBlockColor  (0.65f, 0.08f, 0.08f, 1.f);
static const FLinearColor ResponseOverlapColor(0.72f, 0.62f, 0.00f, 1.f); // lighter than standard
static const FLinearColor ResponseIgnoreColor (0.45f, 0.45f, 0.45f, 1.f); // lighter than standard

// Interaction table — experiment-specific only
static const FLinearColor InteractionSepBgColor(0.08f, 0.08f, 0.08f, 1.f);

// Fixed column widths for the interaction table (FCCPanelStyle::CellW matches FCCPanelStyle::CellW)
static constexpr float ColPresetW  = 200.f;
static constexpr float ColObjTypeW = 200.f;
static constexpr float ColResultH  =  22.f;  // result cell height

static FText CollisionEnabledDisplayName(ECollisionEnabled::Type CE)
{
    switch (CE)
    {
    case ECollisionEnabled::NoCollision:     return NSLOCTEXT("CollisionCommander", "CENoCollision",     "No Collision");
    case ECollisionEnabled::QueryOnly:       return NSLOCTEXT("CollisionCommander", "CEQueryOnly",       "Query Only");
    case ECollisionEnabled::PhysicsOnly:     return NSLOCTEXT("CollisionCommander", "CEPhysicsOnly",     "Physics Only");
    case ECollisionEnabled::QueryAndPhysics: return NSLOCTEXT("CollisionCommander", "CEQueryAndPhysics", "Query and Physics");
    default:                                 return NSLOCTEXT("CollisionCommander", "CEUnknown",         "Unknown");
    }
}

// ============================================================================
//  Construct
// ============================================================================
void SCollisionExperimentPanel::Construct(const FArguments& InArgs)
{
    OnExperimentChanged = InArgs._OnExperimentChanged;
    OnCreateRequested   = InArgs._OnCreateRequested;

    SAssignNew(ChannelContentBox,   SBox);
    SAssignNew(InteractionAreaBox,  SBox);

    // Populate CollisionEnabled options (fixed set — no refresh needed).
    CollisionEnabledOptions = {
        MakeShared<ECollisionEnabled::Type>(ECollisionEnabled::NoCollision),
        MakeShared<ECollisionEnabled::Type>(ECollisionEnabled::QueryOnly),
        MakeShared<ECollisionEnabled::Type>(ECollisionEnabled::PhysicsOnly),
        MakeShared<ECollisionEnabled::Type>(ECollisionEnabled::QueryAndPhysics),
    };

    // Initial placeholder shown until Refresh() populates the grid.
    InteractionAreaBox->SetContent(
        SNew(SBox)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        .Padding(FMargin(24.f))
        [
            SNew(STextBlock)
            .Text(LOCTEXT("InteractionPlaceholder",
                "Configure a preset above to see interactions."))
            .Justification(ETextJustify::Center)
            .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.f)))
        ]
    );

    // Bold font used for all section labels throughout this panel.
    const FSlateFontInfo BoldFont = FCoreStyle::GetDefaultFontStyle("Bold", 9);

    ChildSlot
    [
        SNew(SScrollBox)
        .Orientation(Orient_Vertical)
        .AllowOverscroll(EAllowOverscroll::No)
        + SScrollBox::Slot()
        [
        SNew(SVerticalBox)

        // ── 1. Load from existing ──────────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(8.f, 8.f, 8.f, 4.f))
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(FMargin(8.f, 6.f))
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(FMargin(0.f, 0.f, 8.f, 0.f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("LoadFromLabel", "Load from existing:"))
                    .Font(BoldFont)
                ]

                + SHorizontalBox::Slot()
                .FillWidth(1.f)
                [
                    SAssignNew(SourceCombo, SComboBox<TSharedPtr<FName>>)
                    .OptionsSource(&AllPresetOptions)
                    .OnGenerateWidget_Lambda([](TSharedPtr<FName> Item) -> TSharedRef<SWidget>
                    {
                        const FName& Name = Item.IsValid() ? *Item : NAME_None;
                        const FText  Text = (Name == NewPresetSentinel)
                            ? LOCTEXT("NewPresetItem", "\u2014 New Preset \u2014")
                            : FText::FromName(Name);
                        return SNew(STextBlock)
                            .Text(Text)
                            .Margin(FMargin(4.f, 2.f));
                    })
                    .OnSelectionChanged_Lambda([this](TSharedPtr<FName> Item, ESelectInfo::Type)
                    {
                        if (!bSuppressCallbacks)
                        {
                            LoadFromSelected(Item);
                        }
                    })
                    [
                        SNew(STextBlock)
                        .Text_Lambda([this]() -> FText
                        {
                            const TSharedPtr<FName> Sel = SourceCombo.IsValid()
                                ? SourceCombo->GetSelectedItem() : nullptr;
                            if (!Sel.IsValid() || *Sel == NewPresetSentinel)
                            {
                                return LOCTEXT("SelectSourceHint", "\u2014 New Preset \u2014");
                            }
                            return FText::FromName(*Sel);
                        })
                        .Margin(FMargin(4.f, 2.f))
                    ]
                ]
            ]
        ]

        // ── 2. Preset identity (Name + Object Type) ───────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(8.f, 0.f, 8.f, 4.f))
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(FMargin(8.f, 6.f))
            [
                SNew(SGridPanel)
                .FillColumn(1, 1.f)

                // Row 0: Preset Name
                + SGridPanel::Slot(0, 0)
                .VAlign(VAlign_Center)
                .Padding(FMargin(0.f, 0.f, 12.f, 4.f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("NameLabel", "Preset Name:"))
                    .Font(BoldFont)
                ]
                + SGridPanel::Slot(1, 0)
                .Padding(FMargin(0.f, 0.f, 0.f, 4.f))
                [
                    SNew(SBorder)
                    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                    .Padding(FMargin(6.f, 3.f))
                    [
                        SAssignNew(NameField, SEditableText)
                        .HintText(LOCTEXT("NameHint", "Enter preset name..."))
                        .OnTextChanged_Lambda([this](const FText& NewText)
                        {
                            // Name is metadata only — does not affect collision
                            // computation. Update without firing OnExperimentChanged
                            // so the interaction table isn't rebuilt on every keystroke.
                            ExperimentPreset.PresetName = FName(*NewText.ToString());
                        })
                    ]
                ]

                // Row 1: Object Type
                + SGridPanel::Slot(0, 1)
                .VAlign(VAlign_Center)
                .Padding(FMargin(0.f, 0.f, 12.f, 4.f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("ObjTypeLabel", "Object Type:"))
                    .Font(BoldFont)
                ]
                + SGridPanel::Slot(1, 1)
                .Padding(FMargin(0.f, 0.f, 0.f, 4.f))
                [
                    SAssignNew(ObjectTypeCombo, SComboBox<TSharedPtr<FName>>)
                    .OptionsSource(&AllChannelOptions)
                    .OnGenerateWidget_Lambda([](TSharedPtr<FName> Item) -> TSharedRef<SWidget>
                    {
                        const FString Display = Item.IsValid()
                            ? FCCPanelStyle::FormatChannelDisplayName(*Item)
                            : FString();
                        return SNew(STextBlock)
                            .Text(FText::FromString(Display))
                            .Margin(FMargin(4.f, 2.f));
                    })
                    .OnSelectionChanged_Lambda([this](TSharedPtr<FName> Item, ESelectInfo::Type)
                    {
                        if (!bSuppressCallbacks && Item.IsValid())
                        {
                            ExperimentPreset.ObjectTypeChannel = *Item;
                            OnCollisionDataChanged();
                        }
                    })
                    [
                        SNew(STextBlock)
                        .Text_Lambda([this]() -> FText
                        {
                            return ExperimentPreset.ObjectTypeChannel.IsNone()
                                ? LOCTEXT("SelectObjTypeHint", "— select channel —")
                                : FText::FromString(FCCPanelStyle::FormatChannelDisplayName(
                                    ExperimentPreset.ObjectTypeChannel));
                        })
                        .Margin(FMargin(4.f, 2.f))
                    ]
                ]

                // Row 2: Collision Enabled
                + SGridPanel::Slot(0, 2)
                .VAlign(VAlign_Top)
                .Padding(FMargin(0.f, 4.f, 12.f, 4.f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("CollisionEnabledLabel", "Collision Enabled:"))
                    .Font(BoldFont)
                ]
                + SGridPanel::Slot(1, 2)
                .Padding(FMargin(0.f, 0.f, 0.f, 4.f))
                [
                    SNew(SVerticalBox)

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SAssignNew(CollisionEnabledCombo,
                            SComboBox<TSharedPtr<ECollisionEnabled::Type>>)
                        .OptionsSource(&CollisionEnabledOptions)
                        .OnGenerateWidget_Lambda([](TSharedPtr<ECollisionEnabled::Type> Item)
                            -> TSharedRef<SWidget>
                        {
                            const ECollisionEnabled::Type CE = Item.IsValid()
                                ? *Item : ECollisionEnabled::QueryAndPhysics;
                            return SNew(STextBlock)
                                .Text(CollisionEnabledDisplayName(CE))
                                .Margin(FMargin(4.f, 2.f));
                        })
                        .OnSelectionChanged_Lambda([this](
                            TSharedPtr<ECollisionEnabled::Type> Item, ESelectInfo::Type)
                        {
                            if (!bSuppressCallbacks && Item.IsValid())
                            {
                                ExperimentPreset.CollisionEnabled = *Item;
                                // CollisionEnabled affects which responses are active
                                // — rebuild the interaction table.
                                OnCollisionDataChanged();
                            }
                        })
                        [
                            SNew(STextBlock)
                            .Text_Lambda([this]() -> FText
                            {
                                return CollisionEnabledDisplayName(
                                    ExperimentPreset.CollisionEnabled.GetValue());
                            })
                            .Margin(FMargin(4.f, 2.f))
                        ]
                    ]

                    // Inline warning shown for Query Only and No Collision modes.
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0.f, 3.f, 0.f, 0.f))
                    [
                        SNew(STextBlock)
                        .Text_Lambda([this]() -> FText
                        {
                            const ECollisionEnabled::Type CE =
                                ExperimentPreset.CollisionEnabled.GetValue();
                            if (CE == ECollisionEnabled::NoCollision)
                            {
                                return LOCTEXT("NoCollisionNote",
                                    "All collision responses are ignored — "
                                    "collision is fully disabled.");
                            }
                            return LOCTEXT("QueryOnlyNote",
                                "Block responses will not prevent physics "
                                "bodies from passing through.");
                        })
                        .Visibility_Lambda([this]() -> EVisibility
                        {
                            const ECollisionEnabled::Type CE =
                                ExperimentPreset.CollisionEnabled.GetValue();
                            return (CE == ECollisionEnabled::NoCollision ||
                                    CE == ECollisionEnabled::QueryOnly)
                                ? EVisibility::Visible : EVisibility::Collapsed;
                        })
                        .AutoWrapText(true)
                        .ColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.75f, 0.1f, 1.f)))
                        .Font(FAppStyle::GetFontStyle("SmallFont"))
                    ]
                ]

                // Row 3: Description
                + SGridPanel::Slot(0, 3)
                .VAlign(VAlign_Center)
                .Padding(FMargin(0.f, 0.f, 12.f, 0.f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("DescriptionLabel", "Description:"))
                    .Font(BoldFont)
                ]
                + SGridPanel::Slot(1, 3)
                [
                    SNew(SBorder)
                    .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                    .Padding(FMargin(6.f, 3.f))
                    [
                        SAssignNew(DescriptionField, SEditableText)
                        .HintText(LOCTEXT("DescriptionHint",
                            "Optional description for this preset..."))
                        .OnTextChanged_Lambda([this](const FText& NewText)
                        {
                            ExperimentPreset.Description = NewText.ToString();
                        })
                    ]
                ]
            ]
        ]

        // ── 3. Channel responses header ───────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(8.f, 0.f, 8.f, 0.f))
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(FMargin(8.f, 4.f))
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("ChannelResponsesHeader", "RESPONSES TO EACH OBJECT TYPE"))
                    .Font(BoldFont)
                    .ColorAndOpacity(FSlateColor(FLinearColor(0.75f, 0.75f, 0.75f, 1.f)))
                ]

                + SHorizontalBox::Slot()
                .FillWidth(1.f)

                // Query toggle
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(0.f, 0.f, 2.f, 0.f))
                [
                    SNew(SCheckBox)
                    .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                    .IsChecked_Lambda([this]() -> ECheckBoxState
                    {
                        return bUseQueryResponses ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                    })
                    .OnCheckStateChanged_Lambda([this](ECheckBoxState)
                    {
                        if (!bUseQueryResponses)
                        {
                            bUseQueryResponses = true;
                            RebuildInteractionTable();
                        }
                    })
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("QueryToggle", "Query"))
                        .Margin(FMargin(5.f, 1.f))
                    ]
                ]

                // Physics toggle
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SCheckBox)
                    .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                    .IsChecked_Lambda([this]() -> ECheckBoxState
                    {
                        return !bUseQueryResponses ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                    })
                    .OnCheckStateChanged_Lambda([this](ECheckBoxState)
                    {
                        if (bUseQueryResponses)
                        {
                            bUseQueryResponses = false;
                            RebuildInteractionTable();
                        }
                    })
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("PhysicsToggle", "Physics"))
                        .Margin(FMargin(5.f, 1.f))
                    ]
                ]
            ]
        ]

        // ── 4. Channel response grid (no MaxHeight — shows all channels) ──
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(8.f, 1.f, 8.f, 4.f))
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(FMargin(4.f))
            [
                ChannelContentBox.ToSharedRef()
            ]
        ]

        // ── 5. Interaction area (replaced by live table on every edit) ────
        + SVerticalBox::Slot()
        .FillHeight(1.f)
        .Padding(FMargin(8.f, 0.f, 8.f, 4.f))
        [
            InteractionAreaBox.ToSharedRef()
        ]

        // ── 6. Button bar ─────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(8.f, 4.f, 8.f, 8.f))
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .FillWidth(1.f)

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
                .OnClicked_Lambda([this]()
                {
                    OnCreateRequested.ExecuteIfBound();
                    return FReply::Handled();
                })
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("CreatePresetBtn", "Create Preset"))
                    .Margin(FMargin(10.f, 3.f))
                ]
            ]
        ]
        ] // end SVerticalBox (SScrollBox::Slot content)
    ];    // end SScrollBox / ChildSlot

    Refresh();
}

// ============================================================================
//  Refresh — re-reads snapshot and rebuilds option lists + channel grid
// ============================================================================
void SCollisionExperimentPanel::Refresh()
{
    BaseSnapshot = FCCDataReader::BuildSnapshot();
    RebuildOptions();
    RebuildChannelGrid();
    RebuildInteractionTable();
}

// ============================================================================
//  SetUseQueryResponses
// ============================================================================
void SCollisionExperimentPanel::SetUseQueryResponses(bool bQuery)
{
    if (bUseQueryResponses != bQuery)
    {
        bUseQueryResponses = bQuery;
        RebuildInteractionTable();
        // Channel grid combo labels update automatically via Text_Lambda.
    }
}

// ============================================================================
//  RebuildOptions — repopulates AllPresetOptions and AllChannelOptions
// ============================================================================
void SCollisionExperimentPanel::RebuildOptions()
{
    // Preset options: sentinel first, then all snapshot presets.
    AllPresetOptions.Reset();
    AllPresetOptions.Add(MakeShared<FName>(NewPresetSentinel));
    for (const FCCPresetData& P : BaseSnapshot.Presets)
    {
        AllPresetOptions.Add(MakeShared<FName>(P.PresetName));
    }

    // Channel options: all named channels.
    AllChannelOptions.Reset();
    for (const FCCChannelInfo& Ch : BaseSnapshot.Channels)
    {
        AllChannelOptions.Add(MakeShared<FName>(Ch.Name));
    }

    if (SourceCombo.IsValid())      { SourceCombo->RefreshOptions(); }
    if (ObjectTypeCombo.IsValid())  { ObjectTypeCombo->RefreshOptions(); }

    // Restore object type selection if the channel still exists.
    if (ObjectTypeCombo.IsValid() && !ExperimentPreset.ObjectTypeChannel.IsNone())
    {
        for (const TSharedPtr<FName>& Opt : AllChannelOptions)
        {
            if (Opt.IsValid() && *Opt == ExperimentPreset.ObjectTypeChannel)
            {
                bSuppressCallbacks = true;
                ObjectTypeCombo->SetSelectedItem(Opt);
                bSuppressCallbacks = false;
                break;
            }
        }
    }

    // If ExperimentPreset has no channels yet (first run), initialise it.
    if (ExperimentPreset.QueryResponses.IsEmpty() && !BaseSnapshot.Channels.IsEmpty())
    {
        ExperimentPreset.InitFromSnapshot(BaseSnapshot);
    }
}

// ============================================================================
//  RebuildChannelGrid
//
//  Replaces ChannelContentBox with a 3-column grid of channel response entries.
//  Channels are split into two sections:
//    1. GAME CHANNELS  — project-defined (bIsBuiltin = false, index >= 14)
//    2. ENGINE CHANNELS — UE built-in (bIsBuiltin = true, index 0-13)
//  Channel names are formatted for readability via FormatChannelDisplayName.
// ============================================================================
void SCollisionExperimentPanel::RebuildChannelGrid()
{
    const TArray<FCCChannelInfo>& Channels = BaseSnapshot.Channels;

    if (Channels.IsEmpty())
    {
        ChannelContentBox->SetContent(
            SNew(SBox)
            .HAlign(HAlign_Center)
            .Padding(FMargin(0.f, 8.f))
            [
                SNew(STextBlock)
                .Text(LOCTEXT("NoChannels", "No collision channels found."))
                .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.f)))
            ]
        );
        return;
    }

    // Split into game (project) and engine (built-in) channels.
    TArray<const FCCChannelInfo*> GameChannels, EngineChannels;
    for (const FCCChannelInfo& Ch : Channels)
    {
        if (Ch.bIsBuiltin) { EngineChannels.Add(&Ch); }
        else               { GameChannels.Add(&Ch);   }
    }

    const FSlateFontInfo BoldFont = FCoreStyle::GetDefaultFontStyle("Bold", 9);

    // Builds 3 vertical columns of channels filled top-to-bottom (column-major).
    // Channels are distributed: col 0 gets [0..R-1], col 1 [R..2R-1], col 2 [2R..N-1]
    // where R = ceil(N / 3).
    auto BuildSection = [this](const TArray<const FCCChannelInfo*>& Chans,
                               TSharedRef<SVerticalBox> VBox)
    {
        const int32 N = Chans.Num();
        if (N == 0) { return; }

        constexpr int32 NumCols = 3;
        const int32 RowsPerCol  = (N + NumCols - 1) / NumCols;   // ceil(N/3)

        TSharedRef<SHorizontalBox> ColsRow = SNew(SHorizontalBox);

        for (int32 Col = 0; Col < NumCols; ++Col)
        {
            if (Col > 0)
            {
                // Thin vertical divider between columns
                ColsRow->AddSlot()
                .AutoWidth()
                .Padding(FMargin(4.f, 0.f))
                [
                    SNew(SBorder)
                    .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                    .BorderBackgroundColor(FSlateColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.f)))
                    .Padding(FMargin(0.5f, 0.f))
                    [ SNullWidget::NullWidget ]
                ];
            }

            TSharedRef<SVerticalBox> ColBox = SNew(SVerticalBox);

            const int32 StartIdx = Col * RowsPerCol;
            const int32 EndIdx   = FMath::Min(StartIdx + RowsPerCol, N);

            for (int32 i = StartIdx; i < EndIdx; ++i)
            {
                ColBox->AddSlot()
                .AutoHeight()
                .Padding(FMargin(0.f, 0.f, 0.f, 1.f))
                [
                    MakeChannelEntry(Chans[i]->Name)
                ];
            }

            ColsRow->AddSlot()
            .FillWidth(1.f)
            [ ColBox ];
        }

        VBox->AddSlot()
        .AutoHeight()
        [ ColsRow ];
    };

    TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);

    // ── Game channels first ──────────────────────────────────────────────────
    if (!GameChannels.IsEmpty())
    {
        Content->AddSlot()
        .AutoHeight()
        .Padding(FMargin(4.f, 4.f, 0.f, 2.f))
        [
            SNew(STextBlock)
            .Text(LOCTEXT("GameChannelsHeader", "GAME CHANNELS"))
            .Font(BoldFont)
            .ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.f)))
        ];

        BuildSection(GameChannels, Content);
    }

    // ── Engine channels second ───────────────────────────────────────────────
    if (!EngineChannels.IsEmpty())
    {
        const float TopPad = GameChannels.IsEmpty() ? 4.f : 8.f;

        Content->AddSlot()
        .AutoHeight()
        .Padding(FMargin(4.f, TopPad, 0.f, 2.f))
        [
            SNew(STextBlock)
            .Text(LOCTEXT("EngineChannelsHeader", "ENGINE CHANNELS"))
            .Font(BoldFont)
            .ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.f)))
        ];

        BuildSection(EngineChannels, Content);
    }

    ChannelContentBox->SetContent(Content);
}

// ============================================================================
//  MakeChannelEntry — one channel display-name label + response SComboButton
// ============================================================================
TSharedRef<SWidget> SCollisionExperimentPanel::MakeChannelEntry(FName ChannelName)
{
    return SNew(SHorizontalBox)

    // Formatted channel display name
    + SHorizontalBox::Slot()
    .FillWidth(1.f)
    .VAlign(VAlign_Center)
    .Padding(FMargin(4.f, 2.f))
    [
        SNew(STextBlock)
        .Text(FText::FromString(FCCPanelStyle::FormatChannelDisplayName(ChannelName)))
        .Font(FAppStyle::GetFontStyle("SmallFont"))
    ]

    // Response combo button
    + SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(FMargin(0.f, 1.f, 4.f, 1.f))
    [
        SNew(SComboButton)
        .ButtonStyle(FAppStyle::Get(), "FlatButton")
        .HasDownArrow(true)
        .OnGetMenuContent_Lambda([this, ChannelName]() -> TSharedRef<SWidget>
        {
            return MakeResponseMenu(ChannelName);
        })
        .ButtonContent()
        [
            SNew(STextBlock)
            .Text_Lambda([this, ChannelName]() -> FText
            {
                const TMap<FName, ECollisionResponse>& Map = bUseQueryResponses
                    ? ExperimentPreset.QueryResponses
                    : ExperimentPreset.PhysicsResponses;
                const ECollisionResponse* R = Map.Find(ChannelName);
                return FText::FromString(ResponseToString(R ? *R : ECR_Ignore));
            })
            .ColorAndOpacity_Lambda([this, ChannelName]() -> FSlateColor
            {
                const TMap<FName, ECollisionResponse>& Map = bUseQueryResponses
                    ? ExperimentPreset.QueryResponses
                    : ExperimentPreset.PhysicsResponses;
                const ECollisionResponse* R = Map.Find(ChannelName);
                return FSlateColor(ResponseColor(R ? *R : ECR_Ignore));
            })
            .Font(FAppStyle::GetFontStyle("SmallFont"))
            .Margin(FMargin(6.f, 2.f))
        ]
    ];
}

// ============================================================================
//  MakeResponseMenu — popup with Ignore / Overlap / Block options
// ============================================================================
TSharedRef<SWidget> SCollisionExperimentPanel::MakeResponseMenu(FName ChannelName)
{
    struct FOption { ECollisionResponse R; FText Label; };
    const TArray<FOption> Options =
    {
        { ECR_Ignore,  LOCTEXT("IgnoreOpt",  "Ignore")  },
        { ECR_Overlap, LOCTEXT("OverlapOpt", "Overlap") },
        { ECR_Block,   LOCTEXT("BlockOpt",   "Block")   },
    };

    TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);

    for (const FOption& Opt : Options)
    {
        Menu->AddSlot()
        .AutoHeight()
        [
            SNew(SButton)
            .ButtonStyle(FAppStyle::Get(), "FlatButton")
            .HAlign(HAlign_Left)
            .OnClicked_Lambda([this, ChannelName, R = Opt.R]()
            {
                SetChannelResponse(ChannelName, R);
                FSlateApplication::Get().DismissAllMenus();
                return FReply::Handled();
            })
            [
                SNew(STextBlock)
                .Text(Opt.Label)
                .ColorAndOpacity(FSlateColor(ResponseColor(Opt.R)))
                .Margin(FMargin(12.f, 4.f))
            ]
        ];
    }

    return Menu;
}

// ============================================================================
//  LoadFromSelected
//
//  Sentinel → InitFromSnapshot (blank, all Ignore, empty name).
//  Real preset name → LoadFromPreset (copy all values from snapshot).
//  Tracks LoadedFromPresetName so the tab can detect the overwrite case.
// ============================================================================
void SCollisionExperimentPanel::LoadFromSelected(TSharedPtr<FName> SelectedItem)
{
    if (!SelectedItem.IsValid())
    {
        return;
    }

    if (*SelectedItem == NewPresetSentinel)
    {
        ExperimentPreset.InitFromSnapshot(BaseSnapshot);
        LoadedFromPresetName = NAME_None;
    }
    else
    {
        bool bFound = false;
        for (const FCCPresetData& P : BaseSnapshot.Presets)
        {
            if (P.PresetName == *SelectedItem)
            {
                ExperimentPreset.LoadFromPreset(P);
                LoadedFromPresetName = P.PresetName;
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            return;
        }
    }

    // Sync all identity fields to the newly loaded values.
    bSuppressCallbacks = true;

    if (NameField.IsValid())
    {
        NameField->SetText(ExperimentPreset.PresetName.IsNone()
            ? FText::GetEmpty()
            : FText::FromName(ExperimentPreset.PresetName));
    }

    if (DescriptionField.IsValid())
    {
        DescriptionField->SetText(FText::FromString(ExperimentPreset.Description));
    }

    if (ObjectTypeCombo.IsValid() && !ExperimentPreset.ObjectTypeChannel.IsNone())
    {
        for (const TSharedPtr<FName>& Opt : AllChannelOptions)
        {
            if (Opt.IsValid() && *Opt == ExperimentPreset.ObjectTypeChannel)
            {
                ObjectTypeCombo->SetSelectedItem(Opt);
                break;
            }
        }
    }

    if (CollisionEnabledCombo.IsValid())
    {
        const ECollisionEnabled::Type CE = ExperimentPreset.CollisionEnabled.GetValue();
        for (const TSharedPtr<ECollisionEnabled::Type>& Opt : CollisionEnabledOptions)
        {
            if (Opt.IsValid() && *Opt == CE)
            {
                CollisionEnabledCombo->SetSelectedItem(Opt);
                break;
            }
        }
    }

    bSuppressCallbacks = false;

    // Channel grid combo labels update automatically via Text_Lambda.
    // Interaction table must be explicitly rebuilt for new collision data.
    OnCollisionDataChanged();
}

// ============================================================================
//  SetChannelResponse — updates the active response map and notifies
// ============================================================================
void SCollisionExperimentPanel::SetChannelResponse(FName ChannelName, ECollisionResponse R)
{
    TMap<FName, ECollisionResponse>& Map = bUseQueryResponses
        ? ExperimentPreset.QueryResponses
        : ExperimentPreset.PhysicsResponses;

    Map.FindOrAdd(ChannelName) = R;
    OnCollisionDataChanged();
}

// ============================================================================
//  OnCollisionDataChanged — rebuilds the interaction table then notifies the tab
// ============================================================================
void SCollisionExperimentPanel::OnCollisionDataChanged()
{
    RebuildInteractionTable();
    OnExperimentChanged.ExecuteIfBound();
}

// ============================================================================
//  RebuildInteractionTable
//
//  Replaces InteractionAreaBox with a live table showing how ExperimentPreset
//  would interact with every preset in BaseSnapshot.
//
//  Columns (fixed widths):
//    Existing Preset  200px | Object Type  200px | Result  72px
//
//  Result cells use the same matrix-style double-border + hover animation.
//  Entire row has a white-outline hover (like matrix cells).
//  Result cell tooltip shows the per-side contributions and resolved value.
// ============================================================================
void SCollisionExperimentPanel::RebuildInteractionTable()
{
    static const FSlateRoundedBoxBrush RoundedResultBrush(FLinearColor::White, 3.f);

    if (ExperimentPreset.ObjectTypeChannel.IsNone() || BaseSnapshot.Presets.IsEmpty())
    {
        InteractionAreaBox->SetContent(
            SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            .Padding(FMargin(24.f))
            [
                SNew(STextBlock)
                .Text(LOCTEXT("InteractionPlaceholder",
                    "Set an Object Type above to see interactions."))
                .Justification(ETextJustify::Center)
                .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.f)))
            ]
        );
        return;
    }

    const TArray<FCCPairwiseResult> Results =
        FCCDataReader::ComputeExperimentInteractions(ExperimentPreset, BaseSnapshot);

    const FText PresetLabel = ExperimentPreset.PresetName.IsNone()
        ? LOCTEXT("UnnamedPreset", "(unnamed preset)")
        : FText::FromName(ExperimentPreset.PresetName);

    TSharedRef<SScrollBox> DataScroll = SNew(SScrollBox);

    const int32 N = BaseSnapshot.Presets.Num();
    for (int32 i = 0; i < N; ++i)
    {
        // Visual separator between built-in and custom presets.
        if (i == BaseSnapshot.BuiltinPresetCount && BaseSnapshot.BuiltinPresetCount > 0)
        {
            DataScroll->AddSlot()
            .Padding(FMargin(0.f, 3.f, 0.f, 0.f))
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FSlateColor(InteractionSepBgColor))
                .Padding(FMargin(6.f, 2.f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("CustomPresetsLabel", "Custom Presets"))
                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                    .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.f)))
                ]
            ];
        }

        const FCCPresetData&     Existing   = BaseSnapshot.Presets[i];
        const FCCPairwiseResult  ResultPair = Results.IsValidIndex(i) ? Results[i] : FCCPairwiseResult{};
        const ECollisionResponse DisplayResp = bUseQueryResponses
            ? ResultPair.QueryResult : ResultPair.PhysicsResult;

        // Matrix-matching colours for the result cell.
        const FLinearColor CellBgColor =
            (DisplayResp == ECR_Block)   ? FCCPanelStyle::BlockColor  :
            (DisplayResp == ECR_Overlap) ? FCCPanelStyle::OverlapColor :
                                           FCCPanelStyle::IgnoreColor;
        const FString CellText = ResponseToString(DisplayResp);

        // Per-row hover state — shared across outer outline and inner fill.
        TSharedPtr<bool> bRowHovered = MakeShared<bool>(false);

        // Capture values by value for use inside lambdas.
        const FLinearColor RowBgColor = FCCPanelStyle::DataRowBgColor;

        TSharedRef<SBorder> RowRoot = SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            // Outer: white hover outline (transparent when not hovered)
            .BorderBackgroundColor_Lambda([bRowHovered]() -> FSlateColor
            {
                return *bRowHovered
                    ? FLinearColor(1.f, 1.f, 1.f, 0.70f)
                    : FLinearColor(0.f, 0.f, 0.f, 0.f);
            })
            .Padding(1.f)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                // Inner: row background, brightened 45% on hover
                .BorderBackgroundColor_Lambda([bRowHovered, RowBgColor]() -> FSlateColor
                {
                    if (*bRowHovered)
                    {
                        return FLinearColor(
                            FMath::Min(RowBgColor.R * 1.45f, 1.f),
                            FMath::Min(RowBgColor.G * 1.45f, 1.f),
                            FMath::Min(RowBgColor.B * 1.45f, 1.f), 1.f);
                    }
                    return RowBgColor;
                })
                .Padding(FMargin(0.f))
                [
                    SNew(SHorizontalBox)

                    // ── Existing Preset (200px) ───────────────────────────
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    [
                        SNew(SBox)
                        .WidthOverride(ColPresetW)
                        .Padding(FMargin(6.f, 3.f))
                        [
                            SNew(STextBlock)
                            .Text(FText::FromName(Existing.PresetName))
                            .Font(FAppStyle::GetFontStyle("SmallFont"))
                        ]
                    ]

                    // ── Object Type (200px, formatted) ───────────────────
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    [
                        SNew(SBox)
                        .WidthOverride(ColObjTypeW)
                        .Padding(FMargin(6.f, 3.f))
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(
                                FCCPanelStyle::FormatChannelDisplayName(Existing.ObjectTypeChannel)))
                            .Font(FAppStyle::GetFontStyle("SmallFont"))
                            .ColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.55f, 0.55f, 1.f)))
                        ]
                    ]

                    // ── Result cell (72px, matrix style) ─────────────────
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    .Padding(FMargin(2.f, 2.f, 4.f, 2.f))
                    [
                        SNew(SBox)
                        .WidthOverride(FCCPanelStyle::CellW)
                        .HeightOverride(ColResultH)
                        .HAlign(HAlign_Fill)
                        .VAlign(VAlign_Fill)
                        [
                            SNew(SBorder)
                            .BorderImage(&RoundedResultBrush)
                            // Result cell: brightened when row is hovered
                            .BorderBackgroundColor_Lambda([bRowHovered, CellBgColor]() -> FSlateColor
                            {
                                if (*bRowHovered)
                                {
                                    return FLinearColor(
                                        FMath::Min(CellBgColor.R * 1.45f, 1.f),
                                        FMath::Min(CellBgColor.G * 1.45f, 1.f),
                                        FMath::Min(CellBgColor.B * 1.45f, 1.f), 1.f);
                                }
                                return CellBgColor;
                            })
                            .HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            .ToolTipText(BuildResultTooltip(
                                ExperimentPreset, Existing, DisplayResp, bUseQueryResponses))
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString(CellText))
                                .Font(FAppStyle::GetFontStyle("SmallFont"))
                                .ColorAndOpacity_Lambda([bRowHovered]() -> FSlateColor
                                {
                                    return *bRowHovered
                                        ? FLinearColor(1.f, 1.f, 1.f, 1.f)
                                        : FLinearColor(1.f, 1.f, 1.f, 0.85f);
                                })
                                .Justification(ETextJustify::Center)
                            ]
                        ]
                    ]
                ]
            ];

        RowRoot->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda(
            [bRowHovered](const FGeometry&, const FPointerEvent&) { *bRowHovered = true; }));
        RowRoot->SetOnMouseLeave(FSimpleNoReplyPointerEventHandler::CreateLambda(
            [bRowHovered](const FPointerEvent&) { *bRowHovered = false; }));

        DataScroll->AddSlot()
        .Padding(FMargin(0.f, 0.f, 0.f, 1.f))
        [ RowRoot ];
    }

    // Assemble: section label + fixed column headers + scrollable data rows.
    InteractionAreaBox->SetContent(
        SNew(SVerticalBox)

        // Section label
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0.f, 4.f, 0.f, 0.f))
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(FMargin(6.f, 3.f))
            [
                SNew(STextBlock)
                .Text(FText::Format(
                    LOCTEXT("InteractionHeaderFmt",
                        "HOW \"{0}\" INTERACTS WITH EXISTING PRESETS"),
                    PresetLabel))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
                .ColorAndOpacity(FSlateColor(FLinearColor(0.75f, 0.75f, 0.75f, 1.f)))
            ]
        ]

        // Fixed column headers (outside scroll so they stay visible while scrolling)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FSlateColor(FCCPanelStyle::HeaderBgColor))
            .Padding(FMargin(0.f))
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(0.f))
                [
                    SNew(SBox)
                    .WidthOverride(ColPresetW)
                    .Padding(FMargin(6.f, 3.f))
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("ColPreset", "Existing Preset"))
                        .Font(FAppStyle::GetFontStyle("SmallFont"))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.f)))
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(0.f))
                [
                    SNew(SBox)
                    .WidthOverride(ColObjTypeW)
                    .Padding(FMargin(6.f, 3.f))
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("ColObjType", "Object Type"))
                        .Font(FAppStyle::GetFontStyle("SmallFont"))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.f)))
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(2.f, 0.f, 4.f, 0.f))
                [
                    SNew(SBox)
                    .WidthOverride(FCCPanelStyle::CellW)
                    .HAlign(HAlign_Center)
                    .Padding(FMargin(0.f, 3.f))
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("ColResult", "Result"))
                        .Font(FAppStyle::GetFontStyle("SmallFont"))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.f)))
                    ]
                ]
            ]
        ]

        // Scrollable data rows
        + SVerticalBox::Slot()
        .FillHeight(1.f)
        [
            DataScroll
        ]
    );
}

// FormatChannelDisplayName moved to FCCPanelStyle::FormatChannelDisplayName (CCPanelStyle.h)

// ============================================================================
//  BuildResultTooltip
//
//  Constructs the tooltip shown when hovering the result cell in the
//  interaction table. Format (matches the matrix panel):
//
//    Experiment  →  ExistingPreset:  <Exp's response to Existing's ObjectType>
//    ExistingPreset  →  Experiment:  <Existing's response to Exp's ObjectType>
//
//    Resolved:  <min of the two>
// ============================================================================
FText SCollisionExperimentPanel::BuildResultTooltip(
    const FCCExperimentPreset& Exp,
    const FCCPresetData&       Existing,
    ECollisionResponse         Resolved,
    bool                       bQuery)
{
    const TMap<FName, ECollisionResponse>& ExpMap   = bQuery
        ? Exp.QueryResponses   : Exp.PhysicsResponses;
    const TMap<FName, ECollisionResponse>& ExistMap = bQuery
        ? Existing.QueryResponses : Existing.PhysicsResponses;

    const ECollisionResponse* ExpToExist = ExpMap.Find(Existing.ObjectTypeChannel);
    const ECollisionResponse* ExistToExp = ExistMap.Find(Exp.ObjectTypeChannel);

    const FName DisplayExpName = Exp.PresetName.IsNone()
        ? FName(TEXT("(unnamed)")) : Exp.PresetName;

    return FText::Format(
        LOCTEXT("ExperimentCellTooltipFmt",
            "{0}  \u2192  {1}:  {2}\n{1}  \u2192  {0}:  {3}\n\nResolved:  {4}"),
        FText::FromName(DisplayExpName),
        FText::FromName(Existing.PresetName),
        FText::FromString(ExpToExist ? ResponseToString(*ExpToExist) : TEXT("?")),
        FText::FromString(ExistToExp ? ResponseToString(*ExistToExp) : TEXT("?")),
        FText::FromString(ResponseToString(Resolved)));
}

// ============================================================================
//  Helpers
// ============================================================================
FString SCollisionExperimentPanel::ResponseToString(ECollisionResponse R)
{
    switch (R)
    {
        case ECR_Block:   return TEXT("Block");
        case ECR_Overlap: return TEXT("Overlap");
        default:          return TEXT("Ignore");
    }
}

FLinearColor SCollisionExperimentPanel::ResponseColor(ECollisionResponse R)
{
    switch (R)
    {
        case ECR_Block:   return ResponseBlockColor;
        case ECR_Overlap: return ResponseOverlapColor;
        default:          return ResponseIgnoreColor;
    }
}

#undef LOCTEXT_NAMESPACE
