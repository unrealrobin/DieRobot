// Copyright 2026 Paracosm. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "CCTypes.h"

class SScrollBox;

// Fired when the user left-clicks a non-diagonal data cell.
// Params: RowPreset (A), ColPreset (B) — both by FName.
DECLARE_DELEGATE_TwoParams(FOnMatrixCellClicked, FName /*RowPreset*/, FName /*ColPreset*/);

// ============================================================================
//  SCollisionMatrixPanel
//
//  The N×N collision matrix panel. Reads FCCCollisionSnapshot via
//  FCCDataReader::BuildSnapshot() on construction and on Refresh().
//
//  Sticky-header layout (four independently-scrollable quadrants):
//
//    ┌──────────────┬──────────────────────────────┐
//    │  Corner      │  ColHeaderScroll (H-only)    │
//    ├──────────────┼──────────────────────────────┤
//    │  RowHeader-  │  DataVScroll (V)             │
//    │  Scroll (V)  │    └─ DataHScroll (H)        │
//    │              │         └─ DataContainer     │
//    └──────────────┴──────────────────────────────┘
//
//  ColHeaderScroll and DataHScroll are kept in sync every frame by a
//  RegisterActiveTimer callback so column headers always align with data.
//  Same for RowHeaderScroll / DataVScroll (row headers).
// ============================================================================
class SCollisionMatrixPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCollisionMatrixPanel) {}
        SLATE_EVENT(FOnMatrixCellClicked, OnCellClicked)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // Re-reads from UCollisionProfile and rebuilds the grid.
    void Refresh();

    // Called by SCollisionCommanderTab when the shared Query/Physics toggle changes.
    void SetUseQueryResponses(bool bQuery);

private:
    FCCCollisionSnapshot    Snapshot;
    bool                    bUseQueryResponses = true;
    FOnMatrixCellClicked    OnCellClicked;

    // ── Scroll bars (shared with OS-native scrollbar widgets) ─────────────────
    TSharedPtr<SScrollBar>  HScrollBar;
    TSharedPtr<SScrollBar>  VScrollBar;

    // ── Four-quadrant sticky-header containers ────────────────────────────────
    TSharedPtr<SBox>        CornerBox;           // fixed top-left corner
    TSharedPtr<SBox>        ColHeaderContainer;  // content of col header strip
    TSharedPtr<SBox>        RowHeaderContainer;  // content of row header strip
    TSharedPtr<SBox>        DataContainer;       // the data grid

    TSharedPtr<SScrollBox>  ColHeaderScroll;     // H-only; synced to DataHScroll
    TSharedPtr<SScrollBox>  RowHeaderScroll;     // V-only; synced to DataVScroll
    TSharedPtr<SScrollBox>  DataVScroll;         // V scroll for data (external: VScrollBar)
    TSharedPtr<SScrollBox>  DataHScroll;         // H scroll for data (external: HScrollBar)

    // Computed from longest preset name in RebuildGrid(); drives column header height.
    float DynHeaderH = 160.f;

    // Plugin icon displayed in the top-left corner cell.
    // Loaded once in Construct from Resources/Icon128.png.
    TSharedPtr<FSlateDynamicImageBrush> PluginIconBrush;

    // Panel-level mouse wheel: Shift → H-scroll, else → V-scroll.
    // DataVScroll uses ConsumeMouseWheel::Never so all events bubble here,
    // including events from the col/row header areas.
    virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

    // Per-frame callback that keeps ColHeaderScroll / RowHeaderScroll in sync
    // with the data scroll offsets so headers remain aligned while scrolling.
    EActiveTimerReturnType SyncScrollPositions(double InCurrentTime, float InDeltaTime);

    // Rebuilds all four content areas from the current Snapshot.
    void RebuildGrid();

    // Cell widget factories
    TSharedRef<SWidget> MakeCornerCell() const;
    TSharedRef<SWidget> MakeColumnHeaderCell(int32 PresetIndex) const;
    TSharedRef<SWidget> MakeRowHeaderCell(int32 PresetIndex) const;
    TSharedRef<SWidget> MakeDataCell(int32 A, int32 B) const;
    TSharedRef<SWidget> MakeTraceColumnHeaderCell(int32 TraceIdx) const;
    TSharedRef<SWidget> MakeTraceDataCell(int32 PresetRow, int32 TraceIdx) const;

    // Helpers
    static FText    BuildCellTooltip(const FCCCollisionSnapshot& Snap, int32 A, int32 B, bool bQuery);
    static FText    BuildTraceDataCellTooltip(const FCCCollisionSnapshot& Snap, int32 PresetRow, int32 TraceIdx);
    static FString  ResponseToString(ECollisionResponse R);
};
