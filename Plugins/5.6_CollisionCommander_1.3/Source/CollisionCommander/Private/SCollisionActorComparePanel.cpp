// Copyright 2026 Paracosm. All Rights Reserved.

#include "SCollisionActorComparePanel.h"
#include "CCDataReader.h"
#include "CCPanelStyle.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"

#define LOCTEXT_NAMESPACE "CollisionCommander"


// ----------------------------------------------------------------------------
//  Construct
// ----------------------------------------------------------------------------
void SCollisionActorComparePanel::Construct(const FArguments& InArgs)
{
    SAssignNew(CompareListContainer, SScrollBox);

    ChildSlot
    [
        SNew(SVerticalBox)

        // ── Primary preset selector ───────────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(8.f, 8.f, 8.f, 4.f))
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(FMargin(0.f, 0.f, 8.f, 0.f))
            [
                SNew(STextBlock)
                .Text(LOCTEXT("PrimaryLabel", "Primary Preset:"))
            ]

            + SHorizontalBox::Slot()
            .FillWidth(1.f)
            [
                SAssignNew(PrimaryCombo, SComboBox<TSharedPtr<FName>>)
                .OptionsSource(&AllPresetOptions)
                .OnGenerateWidget_Lambda([](TSharedPtr<FName> Item) -> TSharedRef<SWidget>
                {
                    return SNew(STextBlock)
                        .Text(FText::FromName(Item.IsValid() ? *Item : NAME_None))
                        .Margin(FMargin(4.f, 2.f));
                })
                .OnSelectionChanged_Lambda([this](TSharedPtr<FName> Item, ESelectInfo::Type)
                {
                    if (Item.IsValid())
                    {
                        PrimaryPreset = *Item;
                        RebuildCompareList();
                    }
                })
                [
                    SNew(STextBlock)
                    .Text_Lambda([this]() -> FText
                    {
                        return PrimaryPreset.IsNone()
                            ? LOCTEXT("SelectPrimary", "— select a preset —")
                            : FText::FromName(PrimaryPreset);
                    })
                    .Margin(FMargin(4.f, 2.f))
                ]
            ]
        ]

        // ── Add preset button (just below primary selector) ───────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(8.f, 0.f, 8.f, 4.f))
        [
            SAssignNew(AddComboButton, SComboButton)
            .ButtonStyle(FAppStyle::Get(), "FlatButton")
            .HasDownArrow(true)
            .OnGetMenuContent_Lambda([this]() -> TSharedRef<SWidget>
            {
                return MakeAddPresetsMenu();
            })
            .ButtonContent()
            [
                SNew(STextBlock)
                .Text(LOCTEXT("AddPresetBtn", "+ Add Preset"))
                .Margin(FMargin(6.f, 2.f))
            ]
        ]

        // ── Compare list ─────────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .FillHeight(1.f)
        .Padding(FMargin(8.f, 0.f, 8.f, 0.f))
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(4.f)
            [
                CompareListContainer.ToSharedRef()
            ]
        ]

        // ── Button row (Refresh only; Add Preset moved to top) ───────────────
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
                .ButtonStyle(FAppStyle::Get(), "FlatButton")
                .OnClicked_Lambda([this]() { Refresh(); return FReply::Handled(); })
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("RefreshBtn", "Refresh"))
                    .Margin(FMargin(6.f, 2.f))
                ]
            ]
        ]
    ];

    Refresh();
}

// ----------------------------------------------------------------------------
//  Public API
// ----------------------------------------------------------------------------
void SCollisionActorComparePanel::SetUseQueryResponses(bool bQuery)
{
    if (bUseQueryResponses != bQuery)
    {
        bUseQueryResponses = bQuery;
        RebuildCompareList();
    }
}

void SCollisionActorComparePanel::SetValidationResults(const TArray<FCCValidationResult>& Results)
{
    CachedValidation = Results;
    RebuildCompareList();
}

void SCollisionActorComparePanel::SetupComparison(FName Primary, FName Compare)
{
    PrimaryPreset = Primary;
    if (!ComparePresets.Contains(Compare))
    {
        ComparePresets.Add(Compare);
    }

    for (const TSharedPtr<FName>& Option : AllPresetOptions)
    {
        if (Option.IsValid() && *Option == Primary)
        {
            PrimaryCombo->SetSelectedItem(Option);
            break;
        }
    }

    RebuildCompareList();
}

// ----------------------------------------------------------------------------
//  Private — data management
// ----------------------------------------------------------------------------
void SCollisionActorComparePanel::Refresh()
{
    Snapshot = FCCDataReader::BuildSnapshot();
    RebuildPresetOptions();
    RebuildCompareList();
}

void SCollisionActorComparePanel::RebuildPresetOptions()
{
    AllPresetOptions.Reset();
    for (const FCCPresetData& P : Snapshot.Presets)
    {
        AllPresetOptions.Add(MakeShared<FName>(P.PresetName));
    }

    if (PrimaryCombo.IsValid())
    {
        PrimaryCombo->RefreshOptions();

        if (!PrimaryPreset.IsNone())
        {
            for (const TSharedPtr<FName>& Option : AllPresetOptions)
            {
                if (Option.IsValid() && *Option == PrimaryPreset)
                {
                    PrimaryCombo->SetSelectedItem(Option);
                    break;
                }
            }
        }
    }

    // Drop compare presets that no longer exist in the snapshot
    ComparePresets.RemoveAll([this](const FName& Name)
    {
        return FindPresetIndex(Name) == INDEX_NONE;
    });

    if (!PrimaryPreset.IsNone() && FindPresetIndex(PrimaryPreset) == INDEX_NONE)
    {
        PrimaryPreset = NAME_None;
    }
}

void SCollisionActorComparePanel::RebuildCompareList()
{
    CompareListContainer->ClearChildren();

    if (PrimaryPreset.IsNone())
    {
        CompareListContainer->AddSlot()
        [
            SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            .Padding(24.f)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("SelectPrimaryHint",
                    "Select a primary preset above,\nthen add presets to compare against it."))
                .Justification(ETextJustify::Center)
            ]
        ];
        return;
    }

    if (ComparePresets.IsEmpty())
    {
        CompareListContainer->AddSlot()
        [
            SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            .Padding(24.f)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("NoCompareHint",
                    "No presets in the compare list yet.\nUse \"+ Add Preset\" above."))
                .Justification(ETextJustify::Center)
            ]
        ];
        return;
    }

    for (const FName& Name : ComparePresets)
    {
        CompareListContainer->AddSlot()
        .Padding(FMargin(0.f, 0.f, 0.f, 2.f))
        [
            MakeCompareRow(Name)
        ];
    }
}

// ----------------------------------------------------------------------------
//  MakeCompareRow — one row per preset in the compare list
//
//  Layout: [×] | [preset name fill] | [BADGE] | [→B: resp] | [→A: resp] | [⚠]
// ----------------------------------------------------------------------------
TSharedRef<SWidget> SCollisionActorComparePanel::MakeCompareRow(FName PresetName)
{
    static const FSlateRoundedBoxBrush RoundedBadgeBrush(FLinearColor::White, 3.f);

    const int32 PrimIdx    = FindPresetIndex(PrimaryPreset);
    const int32 CompareIdx = FindPresetIndex(PresetName);

    ECollisionResponse QResult = ECR_Ignore;
    ECollisionResponse PResult = ECR_Ignore;
    FString AtoB_Q = TEXT("?"), BtoA_Q = TEXT("?");
    FString AtoB_P = TEXT("?"), BtoA_P = TEXT("?");

    if (PrimIdx != INDEX_NONE && CompareIdx != INDEX_NONE && Snapshot.PresetCount > 0)
    {
        const FCCPairwiseResult& Pair =
            Snapshot.PairwiseMatrix[PrimIdx * Snapshot.PresetCount + CompareIdx];
        QResult = Pair.QueryResult;
        PResult = Pair.PhysicsResult;

        const FCCPresetData& PP = Snapshot.Presets[PrimIdx];
        const FCCPresetData& CP = Snapshot.Presets[CompareIdx];

        const ECollisionResponse* pAtoB_Q = PP.QueryResponses.Find(CP.ObjectTypeChannel);
        const ECollisionResponse* pBtoA_Q = CP.QueryResponses.Find(PP.ObjectTypeChannel);
        const ECollisionResponse* pAtoB_P = PP.PhysicsResponses.Find(CP.ObjectTypeChannel);
        const ECollisionResponse* pBtoA_P = CP.PhysicsResponses.Find(PP.ObjectTypeChannel);

        AtoB_Q = pAtoB_Q ? ResponseToString(*pAtoB_Q) : TEXT("?");
        BtoA_Q = pBtoA_Q ? ResponseToString(*pBtoA_Q) : TEXT("?");
        AtoB_P = pAtoB_P ? ResponseToString(*pAtoB_P) : TEXT("?");
        BtoA_P = pBtoA_P ? ResponseToString(*pBtoA_P) : TEXT("?");
    }

    const ECollisionResponse DisplayResp = bUseQueryResponses ? QResult : PResult;
    const FString AtoB = bUseQueryResponses ? AtoB_Q : AtoB_P;
    const FString BtoA = bUseQueryResponses ? BtoA_Q : BtoA_P;

    const FLinearColor BadgeColor = ResponseColor(DisplayResp);
    const bool bShowOverlapWarning = CachedValidation.ContainsByPredicate(
        [this, PresetName](const FCCValidationResult& R)
        {
            return R.RuleId == FName(TEXT("CS_OVERLAP_EVENT"))
                && R.AffectedPresets.Contains(PrimaryPreset)
                && R.AffectedPresets.Contains(PresetName);
        });

    return SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .Padding(FMargin(4.f, 3.f))
        [
            SNew(SHorizontalBox)

            // Remove button
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(FMargin(0.f, 0.f, 6.f, 0.f))
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), "FlatButton")
                .OnClicked_Lambda([this, PresetName]()
                {
                    RemoveFromCompare(PresetName);
                    return FReply::Handled();
                })
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("\u00D7")))
                    .Margin(FMargin(4.f, 0.f))
                ]
            ]

            // Preset name + object type channel as a compact sub-label
            + SHorizontalBox::Slot()
            .FillWidth(1.f)
            .VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(FText::FromName(PresetName))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(CompareIdx != INDEX_NONE
                        ? FText::Format(LOCTEXT("ObjTypeSubFmt", "({0})"),
                            FText::FromName(Snapshot.Presets[CompareIdx].ObjectTypeChannel))
                        : FText::GetEmpty())
                    .ColorAndOpacity(FSlateColor(FLinearColor(0.55f, 0.55f, 0.55f, 1.f)))
                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                ]
            ]

            // Response badge
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(FMargin(8.f, 0.f))
            [
                SNew(SBorder)
                .BorderImage(&RoundedBadgeBrush)
                .BorderBackgroundColor(FSlateColor(BadgeColor))
                .Padding(FMargin(8.f, 2.f))
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(ResponseToString(DisplayResp)))
                ]
            ]

            // Primary → Compare
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(FMargin(0.f, 0.f, 12.f, 0.f))
            [
                SNew(STextBlock)
                .Text(FText::Format(
                    LOCTEXT("AtoBFmt", "{0} \u2192 {1}:  {2}"),
                    FText::FromName(PrimaryPreset),
                    FText::FromName(PresetName),
                    FText::FromString(AtoB)))
            ]

            // Compare → Primary
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(FMargin(0.f, 0.f, 8.f, 0.f))
            [
                SNew(STextBlock)
                .Text(FText::Format(
                    LOCTEXT("BtoAFmt", "{0} \u2192 {1}:  {2}"),
                    FText::FromName(PresetName),
                    FText::FromName(PrimaryPreset),
                    FText::FromString(BtoA)))
            ]

            // Overlap warning — collapsed unless ECR_Overlap
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(SBox)
                .Visibility(bShowOverlapWarning ? EVisibility::Visible : EVisibility::Collapsed)
                [
                    SNew(SButton)
                    .ButtonStyle(FAppStyle::Get(), "FlatButton")
                    .ToolTipText(LOCTEXT("OverlapWarningTip",
                        "For Overlap events to fire, both actors must have\n"
                        "\"Generate Overlap Events\" enabled on their collision component."))
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("\u26A0")))
                    ]
                ]
            ]
        ];
}

// ----------------------------------------------------------------------------
//  MakeAddPresetsMenu — popup listing presets not yet in primary or compare
// ----------------------------------------------------------------------------
TSharedRef<SWidget> SCollisionActorComparePanel::MakeAddPresetsMenu()
{
    TSharedRef<SScrollBox> MenuList = SNew(SScrollBox);

    bool bAnyAdded = false;
    for (const FCCPresetData& P : Snapshot.Presets)
    {
        const FName Name = P.PresetName;
        if (Name == PrimaryPreset || ComparePresets.Contains(Name))
        {
            continue;
        }

        MenuList->AddSlot()
        [
            SNew(SButton)
            .ButtonStyle(FAppStyle::Get(), "FlatButton")
            .HAlign(HAlign_Left)
            .OnClicked_Lambda([this, Name]()
            {
                AddToCompare(Name);
                return FReply::Handled();
            })
            [
                SNew(STextBlock)
                .Text(FText::FromName(Name))
                .Margin(FMargin(8.f, 3.f))
            ]
        ];
        bAnyAdded = true;
    }

    if (!bAnyAdded)
    {
        MenuList->AddSlot()
        [
            SNew(SBox)
            .Padding(FMargin(12.f, 6.f))
            [
                SNew(STextBlock)
                .Text(LOCTEXT("NoPresetsToAdd", "All presets already added."))
            ]
        ];
    }

    return SNew(SBox)
        .MaxDesiredHeight(300.f)
        [
            MenuList
        ];
}

void SCollisionActorComparePanel::AddToCompare(FName PresetName)
{
    if (!ComparePresets.Contains(PresetName))
    {
        ComparePresets.Add(PresetName);
        RebuildCompareList();
    }
    if (AddComboButton.IsValid())
    {
        AddComboButton->SetIsOpen(false, false);
    }
}

void SCollisionActorComparePanel::RemoveFromCompare(FName PresetName)
{
    ComparePresets.Remove(PresetName);
    RebuildCompareList();
}

// ----------------------------------------------------------------------------
//  Helpers
// ----------------------------------------------------------------------------
int32 SCollisionActorComparePanel::FindPresetIndex(FName PresetName) const
{
    for (int32 i = 0; i < Snapshot.Presets.Num(); ++i)
    {
        if (Snapshot.Presets[i].PresetName == PresetName)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

FString SCollisionActorComparePanel::ResponseToString(ECollisionResponse R)
{
    switch (R)
    {
        case ECR_Block:   return TEXT("Block");
        case ECR_Overlap: return TEXT("Overlap");
        default:          return TEXT("Ignore");
    }
}

FLinearColor SCollisionActorComparePanel::ResponseColor(ECollisionResponse R)
{
    switch (R)
    {
        case ECR_Block:   return FCCPanelStyle::BlockColor;
        case ECR_Overlap: return FCCPanelStyle::OverlapColor;
        default:          return FCCPanelStyle::IgnoreColor;
    }
}

#undef LOCTEXT_NAMESPACE
