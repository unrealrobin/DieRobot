// Copyright 2026 Paracosm. All Rights Reserved.

#include "SCollisionComponentInspectorPanel.h"
#include "CCDataReader.h"
#include "CCPanelStyle.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "PropertyCustomizationHelpers.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"

#define LOCTEXT_NAMESPACE "CollisionCommander"

// Inspector-specific constants (unique names — safe at file scope)
static const float ColActorBand = 110.f;
static const float ColComponent = 150.f;
static const float ColEnabled   =  60.f;
static const float ColObjType   = 120.f;
static const FLinearColor CellMissingColor(0.12f, 0.12f, 0.12f, 1.f);
static const FLinearColor BannerBgColor   (0.18f, 0.18f, 0.18f, 1.f);  // grey grouping band
static constexpr float    GroupSepPad = 6.f;   // gap between column groups (matches Matrix SepPad)

// ============================================================================
//  Construct
// ============================================================================
void SCollisionComponentInspectorPanel::Construct(const FArguments& InArgs)
{
    OnComponentDataRefreshed = InArgs._OnComponentDataRefreshed;
    ActorEntries.Add(FActorEntry{});   // always start with one empty slot

    // Horizontal scrollbar displayed below the table border.
    SAssignNew(TableHScrollBar, SScrollBar)
        .Orientation(Orient_Horizontal)
        .Thickness(FVector2D(6.f, 6.f));

    // Vertical scroll for rows.  ConsumeMouseWheel::Never so wheel events
    // bubble to this widget's OnMouseWheel override rather than being swallowed
    // here — that lets us route Shift+Scroll to the horizontal scroller.
    SAssignNew(TableContainer, SScrollBox)
        .ConsumeMouseWheel(EConsumeMouseWheel::Never);

    // Horizontal scroll wrapping the vertical one.
    SAssignNew(TableHScroll, SScrollBox)
        .Orientation(Orient_Horizontal)
        .ExternalScrollbar(TableHScrollBar)
        .AllowOverscroll(EAllowOverscroll::No)
        + SScrollBox::Slot()[ TableContainer.ToSharedRef() ];

    SAssignNew(TargetChipsBox,  SHorizontalBox);
    SAssignNew(ActorPickerList, SVerticalBox);

    ChildSlot
    [
        SNew(SVerticalBox)

        // ── Actor picker list ─────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(8.f, 8.f, 8.f, 4.f))
        [
            ActorPickerList.ToSharedRef()
        ]

        // ── Target preset row ─────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(8.f, 0.f, 8.f, 4.f))
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(FMargin(0.f, 0.f, 6.f, 0.f))
            [
                SNew(STextBlock)
                .Text(LOCTEXT("CompareAgainst", "Compare against:"))
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(FMargin(0.f, 0.f, 6.f, 0.f))
            [
                SAssignNew(AddTargetButton, SComboButton)
                .ButtonStyle(FAppStyle::Get(), "FlatButton")
                .HasDownArrow(true)
                .OnGetMenuContent_Lambda([this]() -> TSharedRef<SWidget>
                {
                    return MakeAddPresetsMenu();
                })
                .ButtonContent()
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("AddTargetBtn", "+"))
                    .Margin(FMargin(6.f, 2.f))
                ]
            ]

            + SHorizontalBox::Slot()
            .FillWidth(1.f)
            .VAlign(VAlign_Center)
            [
                SNew(SScrollBox)
                .Orientation(Orient_Horizontal)
                + SScrollBox::Slot()
                [
                    TargetChipsBox.ToSharedRef()
                ]
            ]
        ]

        // ── Refresh button ────────────────────────────────────────────────────
        + SVerticalBox::Slot()
        .AutoHeight()
        .HAlign(HAlign_Right)
        .Padding(FMargin(8.f, 0.f, 8.f, 4.f))
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(0.f)
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .OnClicked_Lambda([this]() { Refresh(); return FReply::Handled(); })
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("RefreshBtn", "Refresh"))
                    .Margin(FMargin(8.f, 3.f))
                ]
            ]
        ]

        // ── Component table ───────────────────────────────────────────────────
        // Outer GroupBorder + inner dark SBorder whose colour is PanelBgColor.
        // TableHScroll (H) wraps TableContainer (V) so both axes are scrollable.
        // The external TableHScrollBar sits below the border.
        + SVerticalBox::Slot()
        .FillHeight(1.f)
        .Padding(FMargin(8.f, 0.f, 8.f, 0.f))
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(0.f)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FCCPanelStyle::PanelBgColor)
                .Padding(1.f)
                [
                    TableHScroll.ToSharedRef()
                ]
            ]
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(8.f, 0.f, 8.f, 8.f))
        [
            TableHScrollBar.ToSharedRef()
        ]
    ];

    RebuildActorPickerList();
    Refresh();
}

// ============================================================================
//  Public API
// ============================================================================
void SCollisionComponentInspectorPanel::SetUseQueryResponses(bool bQuery)
{
    if (bUseQueryResponses != bQuery)
    {
        bUseQueryResponses = bQuery;
        RebuildTable();
    }
}

void SCollisionComponentInspectorPanel::Refresh()
{
    Snapshot = FCCDataReader::BuildSnapshot();

    for (FActorEntry& Entry : ActorEntries)
    {
        Entry.Data = FCCDataReader::BuildActorComponentData(ResolveActor(Entry));
    }

    TargetPresets.RemoveAll([this](const FName& Name)
    {
        return FindPresetData(Name) == nullptr;
    });

    RebuildTargetChips();
    RebuildTable();

    // Notify the tab so it can run RunComponentChecks and push results to
    // the Validation panel.
    if (OnComponentDataRefreshed.IsBound())
    {
        TArray<FCCActorComponentData> AllData;
        for (const FActorEntry& Entry : ActorEntries)
        {
            if (!Entry.ObjectPath.IsEmpty())
            {
                AllData.Add(Entry.Data);
            }
        }
        OnComponentDataRefreshed.Execute(AllData);
    }
}

// ============================================================================
//  Actor management
// ============================================================================
void SCollisionComponentInspectorPanel::AddActorEntry()
{
    ActorEntries.Add(FActorEntry{});
    RebuildActorPickerList();
    // No table rebuild — new entry has no data yet
}

void SCollisionComponentInspectorPanel::RemoveActorEntry(int32 Index)
{
    if (!ActorEntries.IsValidIndex(Index)) return;

    if (ActorEntries.Num() == 1)
    {
        // Keep the last slot — just clear it
        ActorEntries[0] = FActorEntry{};
    }
    else
    {
        ActorEntries.RemoveAt(Index);
    }

    RebuildActorPickerList();
    RebuildTable();
}

void SCollisionComponentInspectorPanel::OnActorPicked(const FAssetData& AssetData, int32 EntryIndex)
{
    if (!ActorEntries.IsValidIndex(EntryIndex)) return;

    FActorEntry& Entry = ActorEntries[EntryIndex];
    if (!AssetData.IsValid())
    {
        Entry = FActorEntry{};
        RebuildTable();
        return;
    }

    Entry.ObjectPath = AssetData.GetObjectPathString();
    Entry.Object     = AssetData.GetAsset();
    Entry.Data       = FCCDataReader::BuildActorComponentData(ResolveActor(Entry));
    RebuildTable();
}

AActor* SCollisionComponentInspectorPanel::ResolveActor(const FActorEntry& Entry) const
{
    UObject* Obj = Entry.Object.Get();
    if (!Obj) return nullptr;

    if (UBlueprint* BP = Cast<UBlueprint>(Obj))
    {
        return BP->GeneratedClass ? Cast<AActor>(BP->GeneratedClass->GetDefaultObject()) : nullptr;
    }
    return Cast<AActor>(Obj);
}

FString SCollisionComponentInspectorPanel::GetEntryDisplayName(const FActorEntry& Entry) const
{
    if (UBlueprint* BP = Cast<UBlueprint>(Entry.Object.Get()))
    {
        return BP->GetName();
    }
    if (AActor* Actor = Cast<AActor>(Entry.Object.Get()))
    {
        return Actor->GetActorLabel();
    }

    // Fallback: extract short name from the object path
    if (!Entry.ObjectPath.IsEmpty())
    {
        FString Name = Entry.ObjectPath;
        int32 Idx;
        if (Name.FindLastChar('/', Idx)) Name = Name.RightChop(Idx + 1);
        if (Name.FindLastChar('.', Idx)) Name = Name.Left(Idx);
        return Name;
    }
    return TEXT("Unknown Actor");
}

void SCollisionComponentInspectorPanel::RebuildActorPickerList()
{
    ActorPickerList->ClearChildren();

    for (int32 Idx = 0; Idx < ActorEntries.Num(); ++Idx)
    {
        const float BottomPad = (Idx < ActorEntries.Num() - 1) ? 4.f : 0.f;

        ActorPickerList->AddSlot()
        .AutoHeight()
        .Padding(FMargin(0.f, 0.f, 0.f, BottomPad))
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(FMargin(0.f, 0.f, 6.f, 0.f))
            [
                SNew(STextBlock)
                .Text(LOCTEXT("ActorPickerLabel", "Actor:"))
            ]

            // SObjectPropertyEntryBox — standard UE asset-reference widget.
            // Supports the arrow (use selected from content browser), browse
            // button, drag-and-drop, and clear. AllowedClass = UBlueprint so
            // the picker shows Blueprint assets rather than world instances.
            + SHorizontalBox::Slot()
            .FillWidth(1.f)
            [
                SNew(SObjectPropertyEntryBox)
                .AllowedClass(UBlueprint::StaticClass())
                .ObjectPath(TAttribute<FString>::CreateLambda([this, Idx]() -> FString
                {
                    return ActorEntries.IsValidIndex(Idx) ? ActorEntries[Idx].ObjectPath : FString();
                }))
                .OnObjectChanged_Lambda([this, Idx](const FAssetData& AssetData)
                {
                    OnActorPicked(AssetData, Idx);
                })
                .AllowClear(true)
                .DisplayUseSelected(true)
                .DisplayBrowse(true)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(FMargin(4.f, 0.f, 0.f, 0.f))
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), "FlatButton")
                .OnClicked_Lambda([this, Idx]()
                {
                    RemoveActorEntry(Idx);
                    return FReply::Handled();
                })
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("\u00D7")))
                    .Margin(FMargin(3.f, 0.f))
                ]
            ]
        ];
    }

    // "Add Actor" button — always at the bottom of the picker list
    ActorPickerList->AddSlot()
    .AutoHeight()
    .Padding(FMargin(0.f, 4.f, 0.f, 0.f))
    .HAlign(HAlign_Left)
    [
        SNew(SButton)
        .ButtonStyle(FAppStyle::Get(), "FlatButton")
        .OnClicked_Lambda([this]()
        {
            AddActorEntry();
            return FReply::Handled();
        })
        [
            SNew(STextBlock)
            .Text(LOCTEXT("AddActorBtn", "+ Add Actor"))
            .Margin(FMargin(2.f, 1.f))
        ]
    ];
}

// ============================================================================
//  Target preset management
// ============================================================================
void SCollisionComponentInspectorPanel::AddTargetPreset(FName PresetName)
{
    if (!TargetPresets.Contains(PresetName))
    {
        TargetPresets.Add(PresetName);
        RebuildTargetChips();
        RebuildTable();
    }
    if (AddTargetButton.IsValid())
    {
        AddTargetButton->SetIsOpen(false, false);
    }
}

void SCollisionComponentInspectorPanel::RemoveTargetPreset(FName PresetName)
{
    TargetPresets.Remove(PresetName);
    RebuildTargetChips();
    RebuildTable();
}

void SCollisionComponentInspectorPanel::RebuildTargetChips()
{
    TargetChipsBox->ClearChildren();

    for (const FName& Target : TargetPresets)
    {
        TargetChipsBox->AddSlot()
        .AutoWidth()
        .Padding(FMargin(0.f, 0.f, 4.f, 0.f))
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .Padding(FMargin(4.f, 2.f))
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(FText::FromName(Target))
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(FMargin(4.f, 0.f, 0.f, 0.f))
                [
                    SNew(SButton)
                    .ButtonStyle(FAppStyle::Get(), "FlatButton")
                    .OnClicked_Lambda([this, Target]()
                    {
                        RemoveTargetPreset(Target);
                        return FReply::Handled();
                    })
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("\u00D7")))
                        .Margin(FMargin(2.f, 0.f))
                    ]
                ]
            ]
        ];
    }
}

TSharedRef<SWidget> SCollisionComponentInspectorPanel::MakeAddPresetsMenu()
{
    TSharedRef<SScrollBox> MenuList = SNew(SScrollBox);

    bool bAnyAdded = false;
    for (const FCCPresetData& P : Snapshot.Presets)
    {
        const FName Name = P.PresetName;
        if (TargetPresets.Contains(Name)) continue;

        MenuList->AddSlot()
        [
            SNew(SButton)
            .ButtonStyle(FAppStyle::Get(), "FlatButton")
            .HAlign(HAlign_Left)
            .OnClicked_Lambda([this, Name]()
            {
                AddTargetPreset(Name);
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
        [ MenuList ];
}

// ============================================================================
//  Table construction
// ============================================================================
void SCollisionComponentInspectorPanel::RebuildTable()
{
    TableContainer->ClearChildren();

    // Build the column-component list. Columns are:
    //   1. Every collision-capable component across every loaded actor, in
    //      ActorEntries order (= add order). Each component becomes one column.
    //   2. Every manually-added preset chip in TargetPresets, appended after
    //      all component columns.
    // The pairwise math is unchanged — each cell still resolves
    // ResolvePair(RowComponent.Preset, ColumnComponent.Preset). We're only
    // changing where the column "Preset" comes from.
    ColumnComponents.Reset();

    for (int32 ActorIdx = 0; ActorIdx < ActorEntries.Num(); ++ActorIdx)
    {
        const FActorEntry& Entry = ActorEntries[ActorIdx];
        if (Entry.ObjectPath.IsEmpty()) continue;

        for (const FCCComponentInfo& Comp : Entry.Data.Components)
        {
            FColumnComponent Col;
            Col.ActorEntryIndex   = ActorIdx;
            Col.ComponentName     = Comp.ComponentName;
            Col.ComponentClass    = Comp.ComponentClass;
            Col.PresetName        = Comp.PresetName;
            Col.ObjectTypeChannel = Comp.ObjectTypeChannel;
            Col.bIsCustomPreset   = (Comp.PresetName == FName(TEXT("Custom")));
            ColumnComponents.Add(MoveTemp(Col));
        }
    }

    for (const FName& Preset : TargetPresets)
    {
        FColumnComponent Col;
        Col.ActorEntryIndex = -1;
        Col.PresetName      = Preset;
        if (const FCCPresetData* PD = FindPresetData(Preset))
        {
            Col.ObjectTypeChannel = PD->ObjectTypeChannel;
        }
        Col.bIsCustomPreset = false;   // chip columns are explicit named presets
        ColumnComponents.Add(MoveTemp(Col));
    }

    const bool bAnySelected = ActorEntries.ContainsByPredicate(
        [](const FActorEntry& E) { return !E.ObjectPath.IsEmpty(); });

    if (!bAnySelected)
    {
        TableContainer->AddSlot()
        [
            SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            .Padding(24.f)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("NoActorHint",
                    "Select a Blueprint actor in the Content Browser, or drag one in."))
                .Justification(ETextJustify::Center)
            ]
        ];
        return;
    }

    // Banner row — actor group labels above the column headers
    TableContainer->AddSlot()
    .Padding(FMargin(0.f, 0.f, 0.f, 1.f))
    [
        MakeColumnBannerRow()
    ];

    // Header row
    TableContainer->AddSlot()
    .Padding(FMargin(0.f, 0.f, 0.f, 1.f))
    [
        MakeHeaderRow()
    ];

    // Data rows — grouped by actor, with left band column
    for (int32 ActorIdx = 0; ActorIdx < ActorEntries.Num(); ++ActorIdx)
    {
        const FActorEntry& Entry = ActorEntries[ActorIdx];
        if (Entry.ObjectPath.IsEmpty()) continue;

        if (Entry.Data.Components.IsEmpty())
        {
            // "No collision-enabled components" hint row with left band
            TableContainer->AddSlot()
            .Padding(FMargin(0.f, 0.f, 0.f, 1.f))
            [
                SNew(SHorizontalBox)

                // Left band cell — actor name
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(0.f, 0.f, 1.f, 0.f))
                [
                    SNew(SBox)
                    .WidthOverride(ColActorBand)
                    .HeightOverride(FCCPanelStyle::CellH)
                    [
                        SNew(SBorder)
                        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                        .BorderBackgroundColor(BannerBgColor)
                        .Padding(FCCPanelStyle::CellPad)
                        [
                            SNew(SBox)
                            .VAlign(VAlign_Center)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString(GetEntryDisplayName(Entry)))
                                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
                                .ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.f)))
                                .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                            ]
                        ]
                    ]
                ]

                // Hint spanning rest of the row
                + SHorizontalBox::Slot()
                .FillWidth(1.f)
                [
                    SNew(SBox)
                    .HAlign(HAlign_Left)
                    .VAlign(VAlign_Center)
                    .Padding(FMargin(8.f, 0.f))
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("NoComponentsHint", "No collision-enabled components."))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.f)))
                    ]
                ]
            ];
            continue;
        }

        for (int32 CompIdx = 0; CompIdx < Entry.Data.Components.Num(); ++CompIdx)
        {
            const bool bIsFirstOfGroup = (CompIdx == 0);
            TableContainer->AddSlot()
            .Padding(FMargin(0.f, 0.f, 0.f, 1.f))
            [
                MakeDataRow(Entry.Data.Components[CompIdx], ActorIdx, bIsFirstOfGroup)
            ];
        }
    }
}

// Banner row: actor-name labels above the column groups + "Presets" for chip columns
TSharedRef<SWidget> SCollisionComponentInspectorPanel::MakeColumnBannerRow() const
{
    TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);

    // Leading spacer — covers: left band + Component + Enabled + ObjType
    const float LeadingWidth = (ColActorBand + 1.f) + (ColComponent + 1.f)
                             + (ColEnabled + 1.f) + (ColObjType + 1.f);

    Row->AddSlot()
    .AutoWidth()
    [
        SNew(SBox)
        .WidthOverride(LeadingWidth)
        .HeightOverride(FCCPanelStyle::CellH)
    ];

    // Trace Channels banner
    if (Snapshot.TraceChannels.Num() > 0)
    {
        const float TraceBannerWidth = Snapshot.TraceChannels.Num() * FCCPanelStyle::CellW;

        Row->AddSlot()
        .AutoWidth()
        [
            SNew(SBox)
            .WidthOverride(TraceBannerWidth)
            .HeightOverride(FCCPanelStyle::CellH)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(BannerBgColor)
                .Padding(FCCPanelStyle::CellPad)
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("TraceChannelsBanner", "Trace Channels"))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.f)))
                        .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                    ]
                ]
            ]
        ];
    }

    // Walk ColumnComponents and emit one banner cell per contiguous actor run
    const bool bHasTraceColumns = (Snapshot.TraceChannels.Num() > 0);
    bool bIsFirstComponentBanner = true;

    int32 RunStart = 0;
    while (RunStart < ColumnComponents.Num())
    {
        const int32 GroupIdx = ColumnComponents[RunStart].ActorEntryIndex;
        int32 RunEnd = RunStart + 1;
        while (RunEnd < ColumnComponents.Num() && ColumnComponents[RunEnd].ActorEntryIndex == GroupIdx)
        {
            ++RunEnd;
        }

        const int32 RunLength = RunEnd - RunStart;
        const float BannerWidth = RunLength * FCCPanelStyle::CellW;

        // Label: actor display name for component groups, "Presets" for chip groups
        FText BannerLabel;
        if (GroupIdx >= 0 && ActorEntries.IsValidIndex(GroupIdx))
        {
            BannerLabel = FText::FromString(GetEntryDisplayName(ActorEntries[GroupIdx]));
        }
        else
        {
            BannerLabel = LOCTEXT("PresetsBanner", "Presets");
        }

        // Separator: gap before this banner if it follows another group
        const float LeftSep = (bIsFirstComponentBanner && bHasTraceColumns)
                            ? GroupSepPad       // gap after trace columns
                            : (bIsFirstComponentBanner ? 0.f : GroupSepPad);

        Row->AddSlot()
        .AutoWidth()
        .Padding(FMargin(LeftSep, 0.f, 0.f, 0.f))
        [
            SNew(SBox)
            .WidthOverride(BannerWidth)
            .HeightOverride(FCCPanelStyle::CellH)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(BannerBgColor)
                .Padding(FCCPanelStyle::CellPad)
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(BannerLabel)
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.f)))
                        .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                    ]
                ]
            ]
        ];

        bIsFirstComponentBanner = false;
        RunStart = RunEnd;
    }

    return Row;
}

// Header row: left band + fixed metadata + trace channels + component/preset columns
TSharedRef<SWidget> SCollisionComponentInspectorPanel::MakeHeaderRow() const
{
    TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);

    auto AddHeaderCell = [&](const FText& Label, float Width, EHorizontalAlignment HAlign = HAlign_Left)
    {
        Row->AddSlot()
        .AutoWidth()
        .Padding(FMargin(0.f, 0.f, 1.f, 0.f))
        [
            SNew(SBox)
            .WidthOverride(Width)
            .HeightOverride(FCCPanelStyle::CellH)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FCCPanelStyle::HeaderBgColor)
                .Padding(FCCPanelStyle::CellPad)
                .ToolTipText(Label)
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign)
                    [
                        SNew(STextBlock)
                        .Text(Label)
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.f)))
                        .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                    ]
                ]
            ]
        ];
    };

    // Left band cell — empty header, same grey as banner
    Row->AddSlot()
    .AutoWidth()
    .Padding(FMargin(0.f, 0.f, 1.f, 0.f))
    [
        SNew(SBox)
        .WidthOverride(ColActorBand)
        .HeightOverride(FCCPanelStyle::CellH)
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(BannerBgColor)
        ]
    ];

    // Fixed metadata columns (Preset column removed — folded into Component tooltip)
    AddHeaderCell(LOCTEXT("HdrComponent", "Component"), ColComponent);
    AddHeaderCell(LOCTEXT("HdrEnabled",   "Enabled"),   ColEnabled,  HAlign_Center);
    AddHeaderCell(LOCTEXT("HdrObjType",   "Obj Type"),  ColObjType);

    // Trace channel columns — orange headers
    for (const FCCChannelInfo& TraceChannel : Snapshot.TraceChannels)
    {
        const FString DisplayName = FCCPanelStyle::FormatChannelDisplayName(TraceChannel.Name);
        Row->AddSlot()
        .AutoWidth()
        [
            SNew(SBox)
            .WidthOverride(FCCPanelStyle::CellW)
            .HeightOverride(FCCPanelStyle::CellH)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FCCPanelStyle::TraceChannelHeaderColor)
                .Padding(0.f)
                .ToolTipText(FText::FromName(TraceChannel.Name))
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(DisplayName))
                        .ColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.92f)))
                        .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                    ]
                ]
            ]
        ];
    }

    // Component + preset columns
    {
    const bool bHasTraceHdr = (Snapshot.TraceChannels.Num() > 0);
    int32 PrevGroupIdx = INT32_MIN;  // sentinel — forces separator on first column if trace exists

    for (int32 ColIdx = 0; ColIdx < ColumnComponents.Num(); ++ColIdx)
    {
        const FColumnComponent& Col = ColumnComponents[ColIdx];

        // Separator at group boundary
        const bool bGroupChanged = (Col.ActorEntryIndex != PrevGroupIdx);
        const float LeftSep = bGroupChanged
            ? ((PrevGroupIdx == INT32_MIN && bHasTraceHdr) ? GroupSepPad   // after trace columns
               : (PrevGroupIdx == INT32_MIN ? 0.f : GroupSepPad))          // between actor groups
            : 0.f;
        PrevGroupIdx = Col.ActorEntryIndex;

        FText HeaderLabel;
        FText HeaderTooltip;

        if (Col.ActorEntryIndex >= 0)
        {
            // Component column — label is component name, tooltip has details
            HeaderLabel = FText::FromName(Col.ComponentName);

            FString ActorName;
            if (ActorEntries.IsValidIndex(Col.ActorEntryIndex))
            {
                ActorName = GetEntryDisplayName(ActorEntries[Col.ActorEntryIndex]);
            }

            if (Col.bIsCustomPreset)
            {
                HeaderTooltip = FText::Format(
                    LOCTEXT("CompColTooltipCustom",
                        "{0}\nClass: {1}\nPreset: {2}  \u26A0\nObject Type: {3}\nActor: {4}\n\n"
                        "\u26A0  This component is not using a named Collision Preset.\n"
                        "Create a preset in Project Settings \u2192 Collision\n"
                        "and assign it to this component."),
                    FText::FromName(Col.ComponentName),
                    FText::FromString(Col.ComponentClass),
                    FText::FromName(Col.PresetName),
                    FText::FromName(Col.ObjectTypeChannel),
                    FText::FromString(ActorName));
            }
            else
            {
                HeaderTooltip = FText::Format(
                    LOCTEXT("CompColTooltip", "{0}\nClass: {1}\nPreset: {2}\nObject Type: {3}\nActor: {4}"),
                    FText::FromName(Col.ComponentName),
                    FText::FromString(Col.ComponentClass),
                    FText::FromName(Col.PresetName),
                    FText::FromName(Col.ObjectTypeChannel),
                    FText::FromString(ActorName));
            }
        }
        else
        {
            // Preset chip column
            HeaderLabel = FText::FromName(Col.PresetName);
            if (const FCCPresetData* PD = FindPresetData(Col.PresetName))
            {
                HeaderTooltip = FText::Format(
                    LOCTEXT("PresetColTooltip", "{0}\nObject type: {1}"),
                    FText::FromName(Col.PresetName),
                    FText::FromName(PD->ObjectTypeChannel));
            }
            else
            {
                HeaderTooltip = FText::FromName(Col.PresetName);
            }
        }

        // Build column header cell — ⚠ inline for custom-preset components
        TSharedRef<SWidget> HeaderContent =
            (Col.ActorEntryIndex >= 0 && Col.bIsCustomPreset)
            ? StaticCastSharedRef<SWidget>(
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .FillWidth(1.f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(HeaderLabel)
                    .ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.f)))
                    .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(FMargin(3.f, 0.f, 0.f, 0.f))
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("\u26A0")))
                    .ColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.75f, 0.1f, 1.f)))
                ]
              )
            : StaticCastSharedRef<SWidget>(
                SNew(STextBlock)
                .Text(HeaderLabel)
                .ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.f)))
                .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
              );

        Row->AddSlot()
        .AutoWidth()
        .Padding(FMargin(LeftSep, 0.f, 0.f, 0.f))
        [
            SNew(SBox)
            .WidthOverride(FCCPanelStyle::CellW)
            .HeightOverride(FCCPanelStyle::CellH)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FCCPanelStyle::HeaderBgColor)
                .Padding(0.f)
                .ToolTipText(HeaderTooltip)
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Center)
                    [
                        HeaderContent
                    ]
                ]
            ]
        ];
    }
    }  // end scope for PrevGroupIdx

    return Row;
}

// Data row: left band + metadata + trace channels + component/preset interaction cells
TSharedRef<SWidget> SCollisionComponentInspectorPanel::MakeDataRow(
    const FCCComponentInfo& Component, int32 RowActorIndex, bool bIsFirstOfGroup) const
{
    TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);

    auto AddMetaCell = [&](const FText& Content, float Width,
                           EHorizontalAlignment HAlign = HAlign_Left,
                           const FText& Tooltip = FText::GetEmpty())
    {
        Row->AddSlot()
        .AutoWidth()
        .Padding(FMargin(0.f, 0.f, 1.f, 0.f))
        [
            SNew(SBox)
            .WidthOverride(Width)
            .HeightOverride(FCCPanelStyle::CellH)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FCCPanelStyle::DataRowBgColor)
                .Padding(FCCPanelStyle::CellPad)
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign)
                    [
                        SNew(STextBlock)
                        .Text(Content)
                        .ToolTipText(Tooltip)
                        .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                    ]
                ]
            ]
        ];
    };

    // ── Left actor band ──────────────────────────────────────────────────────
    {
        FText BandLabel = FText::GetEmpty();
        if (bIsFirstOfGroup && ActorEntries.IsValidIndex(RowActorIndex))
        {
            BandLabel = FText::FromString(GetEntryDisplayName(ActorEntries[RowActorIndex]));
        }

        Row->AddSlot()
        .AutoWidth()
        .Padding(FMargin(0.f, 0.f, 1.f, 0.f))
        [
            SNew(SBox)
            .WidthOverride(ColActorBand)
            .HeightOverride(FCCPanelStyle::CellH)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(BannerBgColor)
                .Padding(FCCPanelStyle::CellPad)
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(BandLabel)
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.f)))
                        .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                    ]
                ]
            ]
        ];
    }

    // ── Component name cell ──────────────────────────────────────────────────
    // Preset column removed — info folded into the Component cell tooltip.
    // ⚠ inline when PresetName == "Custom".
    {
        const bool bIsCustom = (Component.PresetName == FName(TEXT("Custom")));

        FText CompTooltip;
        if (bIsCustom)
        {
            CompTooltip = FText::Format(
                LOCTEXT("CompCellTooltipCustom",
                    "{0}\nClass: {1}\nPreset: {2}  \u26A0\nObject Type: {3}\n\n"
                    "\u26A0  This component is not using a named Collision Preset.\n"
                    "Collision behaviour is harder to audit and maintain.\n\n"
                    "Create a preset in Project Settings \u2192 Collision\n"
                    "and assign it to this component."),
                FText::FromName(Component.ComponentName),
                FText::FromString(Component.ComponentClass),
                FText::FromName(Component.PresetName),
                FText::FromName(Component.ObjectTypeChannel));
        }
        else
        {
            CompTooltip = FText::Format(
                LOCTEXT("CompCellTooltip", "{0}\nClass: {1}\nPreset: {2}\nObject Type: {3}"),
                FText::FromName(Component.ComponentName),
                FText::FromString(Component.ComponentClass),
                FText::FromName(Component.PresetName),
                FText::FromName(Component.ObjectTypeChannel));
        }

        Row->AddSlot()
        .AutoWidth()
        .Padding(FMargin(0.f, 0.f, 1.f, 0.f))
        [
            SNew(SBox)
            .WidthOverride(ColComponent)
            .HeightOverride(FCCPanelStyle::CellH)
            [
                SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FCCPanelStyle::DataRowBgColor)
                .Padding(FCCPanelStyle::CellPad)
                .ToolTipText(CompTooltip)
                [
                    SNew(SBox)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Left)
                    [
                        SNew(SHorizontalBox)

                        + SHorizontalBox::Slot()
                        .FillWidth(1.f)
                        .VAlign(VAlign_Center)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromName(Component.ComponentName))
                            .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
                        ]

                        // ⚠ — only visible on Custom preset
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .Padding(FMargin(5.f, 0.f, 0.f, 0.f))
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(TEXT("\u26A0")))
                            .ColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.75f, 0.1f, 1.f)))
                            .Visibility(bIsCustom ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
                        ]
                    ]
                ]
            ]
        ];
    }

    // ── Enabled + Obj Type metadata ──────────────────────────────────────────
    AddMetaCell(EnabledToText(Component.CollisionEnabled), ColEnabled, HAlign_Center);
    AddMetaCell(FText::FromName(Component.ObjectTypeChannel), ColObjType);

    // ── Trace channel response cells ─────────────────────────────────────────
    {
        static const FSlateRoundedBoxBrush RoundedBrush(FLinearColor::White, 3.f);
        const FCCPresetData* PresetData = FindPresetData(Component.PresetName);

        for (const FCCChannelInfo& TraceChannel : Snapshot.TraceChannels)
        {
            const bool bMissing = (PresetData == nullptr);
            ECollisionResponse TraceResp = ECR_Ignore;
            if (!bMissing)
            {
                const ECollisionResponse* Found = PresetData->QueryResponses.Find(TraceChannel.Name);
                TraceResp = Found ? *Found : ECR_Ignore;
            }

            const FString DisplayName = FCCPanelStyle::FormatChannelDisplayName(TraceChannel.Name);
            const FText CellTooltip = FText::Format(
                LOCTEXT("TraceRespCellTooltip", "{0}  \u2192  {1}:  {2}"),
                FText::FromName(Component.PresetName),
                FText::FromString(DisplayName),
                FText::FromString(ResponseLabel(TraceResp, bMissing).ToString()));

            Row->AddSlot()
            .AutoWidth()
            [
                SNew(SBox)
                .WidthOverride(FCCPanelStyle::CellW)
                .HeightOverride(FCCPanelStyle::CellH)
                [
                    SNew(SBorder)
                    .BorderImage(&RoundedBrush)
                    .BorderBackgroundColor(FLinearColor::Transparent)
                    .Padding(1.f)
                    .ToolTipText(CellTooltip)
                    [
                        SNew(SBorder)
                        .BorderImage(&RoundedBrush)
                        .BorderBackgroundColor(ResponseColor(TraceResp, bMissing))
                        .Padding(FCCPanelStyle::CellPad)
                        [
                            SNew(SBox)
                            .WidthOverride(FCCPanelStyle::CellW - 2.f * FCCPanelStyle::CellPad - 2.f)
                            .HeightOverride(FCCPanelStyle::CellH - 2.f * FCCPanelStyle::CellPad - 2.f)
                            .HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            [
                                SNew(STextBlock)
                                .Text(ResponseLabel(TraceResp, bMissing))
                                .ColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.85f)))
                                .Justification(ETextJustify::Center)
                            ]
                        ]
                    ]
                ]
            ];
        }
    }

    // ── Component + preset interaction cells ─────────────────────────────────
    {
        static const FSlateRoundedBoxBrush RoundedBrush(FLinearColor::White, 3.f);

        const bool bHasTraceData = (Snapshot.TraceChannels.Num() > 0);
        int32 PrevGroupIdx = INT32_MIN;

        for (int32 ColIdx = 0; ColIdx < ColumnComponents.Num(); ++ColIdx)
        {
            const FColumnComponent& Col = ColumnComponents[ColIdx];

            // Separator at group boundary
            const bool bGroupChanged = (Col.ActorEntryIndex != PrevGroupIdx);
            const float LeftSep = bGroupChanged
                ? ((PrevGroupIdx == INT32_MIN && bHasTraceData) ? GroupSepPad
                   : (PrevGroupIdx == INT32_MIN ? 0.f : GroupSepPad))
                : 0.f;
            PrevGroupIdx = Col.ActorEntryIndex;

            // Diagonal: same actor + same component name = self-vs-self → dark blue
            const bool bIsDiagonal = (Col.ActorEntryIndex == RowActorIndex)
                                  && (Col.ComponentName == Component.ComponentName)
                                  && (Col.ActorEntryIndex >= 0);

            if (bIsDiagonal)
            {
                // Diagonal cell — dark blue background but still shows the
                // resolved interaction result, matching the Matrix tab.
                const FCCPresetData* A = FindPresetData(Component.PresetName);
                const bool bMissing = (A == nullptr);
                ECollisionResponse DiagResult = ECR_Ignore;
                if (!bMissing)
                {
                    DiagResult = FCCDataReader::ResolvePair(*A, *A, bUseQueryResponses);
                }

                Row->AddSlot()
                .AutoWidth()
                .Padding(FMargin(LeftSep, 0.f, 0.f, 0.f))
                [
                    SNew(SBox)
                    .WidthOverride(FCCPanelStyle::CellW)
                    .HeightOverride(FCCPanelStyle::CellH)
                    [
                        SNew(SBorder)
                        .BorderImage(&RoundedBrush)
                        .BorderBackgroundColor(FLinearColor::Transparent)
                        .Padding(1.f)
                        .ToolTipText(BuildInteractionCellTooltip(Component.PresetName, Component.PresetName))
                        [
                            SNew(SBorder)
                            .BorderImage(&RoundedBrush)
                            .BorderBackgroundColor(FLinearColor(0.04f, 0.04f, 0.30f, 1.f))
                            .Padding(FCCPanelStyle::CellPad)
                            [
                                SNew(SBox)
                                .WidthOverride(FCCPanelStyle::CellW - 2.f * FCCPanelStyle::CellPad - 2.f)
                                .HeightOverride(FCCPanelStyle::CellH - 2.f * FCCPanelStyle::CellPad - 2.f)
                                .HAlign(HAlign_Center)
                                .VAlign(VAlign_Center)
                                [
                                    SNew(STextBlock)
                                    .Text(ResponseLabel(DiagResult, bMissing))
                                    .ColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.85f)))
                                    .Justification(ETextJustify::Center)
                                ]
                            ]
                        ]
                    ]
                ];
            }
            else
            {
                Row->AddSlot()
                .AutoWidth()
                .Padding(FMargin(LeftSep, 0.f, 0.f, 0.f))
                [
                    MakeInteractionCell(Component.PresetName, Col.PresetName)
                ];
            }
        }
    }

    return Row;
}

// Interaction cell: colour-coded SBorder, full word label, matches matrix style
TSharedRef<SWidget> SCollisionComponentInspectorPanel::MakeInteractionCell(
    FName ComponentPreset, FName TargetPreset) const
{
    // Rounded brush — white fill tinted via BorderBackgroundColor, matching the matrix panel.
    // static const: constructed once, valid for the plugin lifetime.
    static const FSlateRoundedBoxBrush RoundedBrush(FLinearColor::White, 3.f);

    const FCCPresetData* A = FindPresetData(ComponentPreset);
    const FCCPresetData* B = FindPresetData(TargetPreset);
    const bool bMissing    = (!A || !B);

    ECollisionResponse Result = ECR_Ignore;
    if (!bMissing)
    {
        Result = FCCDataReader::ResolvePair(*A, *B, bUseQueryResponses);
    }

    const FLinearColor BgColor = ResponseColor(Result, bMissing);

    return SNew(SBox)
        .WidthOverride(FCCPanelStyle::CellW)
        .HeightOverride(FCCPanelStyle::CellH)
        [
            // Outer border — transparent normally (matches matrix double-border structure)
            SNew(SBorder)
            .BorderImage(&RoundedBrush)
            .BorderBackgroundColor(FLinearColor::Transparent)
            .Padding(1.f)
            .ToolTipText(BuildInteractionCellTooltip(ComponentPreset, TargetPreset))
            [
                // Inner border — fill colour
                SNew(SBorder)
                .BorderImage(&RoundedBrush)
                .BorderBackgroundColor(BgColor)
                .Padding(FCCPanelStyle::CellPad)
                [
                    SNew(SBox)
                    .WidthOverride(FCCPanelStyle::CellW - 2.f * FCCPanelStyle::CellPad - 2.f)
                    .HeightOverride(FCCPanelStyle::CellH - 2.f * FCCPanelStyle::CellPad - 2.f)
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(ResponseLabel(Result, bMissing))
                        .ColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.85f)))
                        .Justification(ETextJustify::Center)
                    ]
                ]
            ]
        ];
}

// Tooltip for an interaction cell — mirrors the matrix panel BuildCellTooltip format:
//   ComponentPreset  →  TargetPreset:  <ComponentPreset's response to Target's ObjectType>
//   TargetPreset     →  ComponentPreset: <TargetPreset's response to Component's ObjectType>
//   Resolved:  Block|Overlap|Ignore
FText SCollisionComponentInspectorPanel::BuildInteractionCellTooltip(
    FName ComponentPreset, FName TargetPreset) const
{
    const FCCPresetData* A = FindPresetData(ComponentPreset);
    const FCCPresetData* B = FindPresetData(TargetPreset);

    if (!A || !B)
    {
        return LOCTEXT("CellTooltipMissing", "One or both presets not found in the current snapshot.");
    }

    const TMap<FName, ECollisionResponse>& AMap = bUseQueryResponses ? A->QueryResponses : A->PhysicsResponses;
    const TMap<FName, ECollisionResponse>& BMap = bUseQueryResponses ? B->QueryResponses : B->PhysicsResponses;

    const ECollisionResponse* AtoBPtr = AMap.Find(B->ObjectTypeChannel);
    const ECollisionResponse* BtoAPtr = BMap.Find(A->ObjectTypeChannel);

    const FString AtoB    = AtoBPtr ? ResponseLabel(*AtoBPtr, false).ToString() : TEXT("?");
    const FString BtoA    = BtoAPtr ? ResponseLabel(*BtoAPtr, false).ToString() : TEXT("?");
    const FString Resolved = ResponseLabel(FCCDataReader::ResolvePair(*A, *B, bUseQueryResponses), false).ToString();

    return FText::Format(
        LOCTEXT("InspectorCellTooltipFmt",
            "{0}  \u2192  {1}:  {2}\n{1}  \u2192  {0}:  {3}\n\nResolved:  {4}"),
        FText::FromName(ComponentPreset),
        FText::FromName(TargetPreset),
        FText::FromString(AtoB),
        FText::FromString(BtoA),
        FText::FromString(Resolved));
}

// ============================================================================
//  Helpers
// ============================================================================
const FCCPresetData* SCollisionComponentInspectorPanel::FindPresetData(FName PresetName) const
{
    for (const FCCPresetData& P : Snapshot.Presets)
    {
        if (P.PresetName == PresetName) return &P;
    }
    return nullptr;
}

FText SCollisionComponentInspectorPanel::EnabledToText(ECollisionEnabled::Type Enabled)
{
    switch (Enabled)
    {
        case ECollisionEnabled::QueryOnly:       return FText::FromString(TEXT("Q"));
        case ECollisionEnabled::PhysicsOnly:     return FText::FromString(TEXT("P"));
        case ECollisionEnabled::QueryAndPhysics: return FText::FromString(TEXT("Q+P"));
        case ECollisionEnabled::ProbeOnly:       return FText::FromString(TEXT("Probe"));
        case ECollisionEnabled::QueryAndProbe:   return FText::FromString(TEXT("Q+Pb"));
        default:                                 return FText::FromString(TEXT("\u2014"));
    }
}

FLinearColor SCollisionComponentInspectorPanel::ResponseColor(ECollisionResponse R, bool bMissing)
{
    if (bMissing) return CellMissingColor;
    switch (R)
    {
        case ECR_Block:   return FCCPanelStyle::BlockColor;
        case ECR_Overlap: return FCCPanelStyle::OverlapColor;
        default:          return FCCPanelStyle::IgnoreColor;
    }
}

FText SCollisionComponentInspectorPanel::ResponseLabel(ECollisionResponse R, bool bMissing)
{
    if (bMissing) return FText::FromString(TEXT("\u2013"));    // en dash: –
    switch (R)
    {
        case ECR_Block:   return LOCTEXT("RespBlock",   "Block");
        case ECR_Overlap: return LOCTEXT("RespOverlap", "Overlap");
        default:          return LOCTEXT("RespIgnore",  "Ignore");
    }
}

// ============================================================================
//  Mouse wheel — panel-level handler
//
//  TableContainer uses ConsumeMouseWheel::Never so all wheel events bubble
//  here regardless of where the cursor sits (table rows, header, controls).
//  Shift held  → horizontal scroll via TableHScroll.
//  No modifier → vertical scroll via TableContainer.
// ============================================================================
FReply SCollisionComponentInspectorPanel::OnMouseWheel(
    const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    const float Delta = MouseEvent.GetWheelDelta();
    if (MouseEvent.IsShiftDown())
    {
        if (TableHScroll.IsValid())
        {
            TableHScroll->SetScrollOffset(TableHScroll->GetScrollOffset() - Delta * 40.f);
            return FReply::Handled();
        }
    }
    else
    {
        if (TableContainer.IsValid())
        {
            TableContainer->SetScrollOffset(TableContainer->GetScrollOffset() - Delta * 40.f);
            return FReply::Handled();
        }
    }
    return SCompoundWidget::OnMouseWheel(MyGeometry, MouseEvent);
}

#undef LOCTEXT_NAMESPACE
