// Copyright 2026 Paracosm. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Math/Color.h"

// Shared visual constants used across all CollisionCommander panel widgets.
// Unity-build safe: header guard ensures the struct is declared only once
// per translation unit regardless of how many .cpp files include it.
struct FCCPanelStyle
{
    // Cell geometry — all panels use these dimensions for alignment
    static constexpr float CellW   = 72.f;   // data cell width (wide enough for "Overlap")
    static constexpr float CellH   = 26.f;   // data cell height
    static constexpr float CellPad =  2.f;   // inner padding per cell edge; 1px slot gap = grid line

    // Response colours — Block / Overlap / Ignore
    inline static const FLinearColor BlockColor  {0.65f, 0.08f, 0.08f, 1.f};
    inline static const FLinearColor OverlapColor{0.52f, 0.46f, 0.00f, 1.f};
    inline static const FLinearColor IgnoreColor {0.20f, 0.20f, 0.20f, 1.f};

    // Background colours — shared chrome
    inline static const FLinearColor HeaderBgColor        {0.10f,  0.10f,  0.10f,  1.f};
    inline static const FLinearColor PanelBgColor         {0.055f, 0.055f, 0.055f, 1.f}; // grid-line gap
    inline static const FLinearColor DataRowBgColor       {0.14f,  0.14f,  0.14f,  1.f};

    // Trace channel column header colour — brand orange #E05C2A
    inline static const FLinearColor TraceChannelHeaderColor{0.878f, 0.361f, 0.165f, 1.f};

    // Converts raw UE internal channel names to readable display strings:
    //   "EngineTraceChannel1" → "Engine Trace 1"
    //   "GameTraceChannel0"   → "Game Trace 0"
    //   Named channels (Pawn, Camera, Visibility, …) are returned unchanged.
    // Shared by Matrix, Inspector, and Experiment panels.
    static FString FormatChannelDisplayName(FName ChannelName)
    {
        if (ChannelName.IsNone()) return FString();

        const FString Str = ChannelName.ToString();

        static const FString EnginePrefix(TEXT("EngineTraceChannel"));
        static const FString GamePrefix  (TEXT("GameTraceChannel"));

        if (Str.StartsWith(EnginePrefix))
        {
            return FString::Printf(TEXT("Engine Trace %s"), *Str.Mid(EnginePrefix.Len()));
        }
        if (Str.StartsWith(GamePrefix))
        {
            return FString::Printf(TEXT("Game Trace %s"), *Str.Mid(GamePrefix.Len()));
        }

        return Str;
    }
};
