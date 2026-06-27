// Copyright 2026 Paracosm. All Rights Reserved.

#include "CollisionCommanderModule.h"
#include "SCollisionCommanderTab.h"
#include "UCollisionCommanderSettings.h"

#include "Modules/ModuleManager.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "ToolMenus.h"
#include "Styling/AppStyle.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IPluginManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#define LOCTEXT_NAMESPACE "CollisionCommander"

const FName FCollisionCommanderModule::TabId("CollisionCommanderTab");

// Populated on the game thread when the version check response arrives.
// Empty = no update available (or check not yet complete).
static FString GAvailableVersion;

// ---------------------------------------------------------------------------
//  Version comparison helper — returns true if Remote is strictly newer
//  than Current. Supports "X.Y" format.
// ---------------------------------------------------------------------------
static bool IsVersionNewer(const FString& Remote, const FString& Current)
{
    auto Parse = [](const FString& V, int32& Maj, int32& Min)
    {
        TArray<FString> Parts;
        V.ParseIntoArray(Parts, TEXT("."));
        Maj = Parts.IsValidIndex(0) ? FCString::Atoi(*Parts[0]) : 0;
        Min = Parts.IsValidIndex(1) ? FCString::Atoi(*Parts[1]) : 0;
    };

    int32 RemMaj, RemMin, CurMaj, CurMin;
    Parse(Remote, RemMaj, RemMin);
    Parse(Current, CurMaj, CurMin);

    if (RemMaj != CurMaj) return RemMaj > CurMaj;
    return RemMin > CurMin;
}

// static
const FString& FCollisionCommanderModule::GetAvailableVersion()
{
    return GAvailableVersion;
}

// ---------------------------------------------------------------------------
//  Tab spawner — called by FGlobalTabmanager when the tab is invoked.
// ---------------------------------------------------------------------------
static TSharedRef<SDockTab> SpawnCollisionCommanderTab(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SCollisionCommanderTab)
        ];
}

// ---------------------------------------------------------------------------
//  Module lifecycle
// ---------------------------------------------------------------------------
void FCollisionCommanderModule::StartupModule()
{
    // Register the nomad tab. SetMenuType(Enabled) makes it appear in the
    // Window menu automatically. SetGroup places it under Window > Level Editor.
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        TabId,
        FOnSpawnTab::CreateStatic(&SpawnCollisionCommanderTab))
        .SetDisplayName(LOCTEXT("TabTitle", "Collision Commander"))
        .SetTooltipText(LOCTEXT("TabTooltip", "View all collision preset interactions in a live matrix"))
        .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Edit"))
        .SetMenuType(ETabSpawnerMenuType::Enabled)
        .SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory());

    // Defer toolbar registration until after ToolMenus is ready.
    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCollisionCommanderModule::RegisterMenus));

    // Version check — one async GET per editor session, silent on failure.
    const UCollisionCommanderSettings* Settings = GetDefault<UCollisionCommanderSettings>();
    if (Settings && Settings->bCheckForUpdates)
    {
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
            FHttpModule::Get().CreateRequest();
        Request->SetURL(TEXT("https://paracosm.gg/collision-commander/version.json"));
        Request->SetVerb(TEXT("GET"));
        Request->OnProcessRequestComplete().BindLambda(
            [](FHttpRequestPtr /*Req*/, FHttpResponsePtr Response, bool bSuccess)
            {
                if (!bSuccess || !Response.IsValid() ||
                    !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
                {
                    return;
                }

                TSharedPtr<FJsonObject> JsonObj;
                TSharedRef<TJsonReader<>> Reader =
                    TJsonReaderFactory<>::Create(Response->GetContentAsString());
                if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
                {
                    return;
                }

                FString RemoteVersion;
                if (!JsonObj->TryGetStringField(TEXT("version"), RemoteVersion) ||
                    RemoteVersion.IsEmpty())
                {
                    return;
                }

                TSharedPtr<IPlugin> Plugin =
                    IPluginManager::Get().FindPlugin(TEXT("CollisionCommander"));
                if (!Plugin.IsValid()) return;

                if (IsVersionNewer(RemoteVersion, Plugin->GetDescriptor().VersionName))
                {
                    GAvailableVersion = RemoteVersion;
                }
            });
        Request->ProcessRequest();
    }
}

void FCollisionCommanderModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);
}

// ---------------------------------------------------------------------------
//  Toolbar button registration
// ---------------------------------------------------------------------------
void FCollisionCommanderModule::RegisterMenus()
{
    UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.AssetsToolBar");
    FToolMenuSection& Section = Toolbar->FindOrAddSection("CollisionCommander");
    Section.AddEntry(FToolMenuEntry::InitToolBarButton(
        "OpenCollisionCommander",
        FUIAction(FExecuteAction::CreateStatic(&FCollisionCommanderModule::OpenCollisionCommanderTab)),
        LOCTEXT("ToolbarLabel", "Collision\nCommander"),
        LOCTEXT("ToolbarTooltip", "Open the Collision Commander panel"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Edit")
    ));
}

void FCollisionCommanderModule::OpenCollisionCommanderTab()
{
    FGlobalTabmanager::Get()->TryInvokeTab(TabId);
}

IMPLEMENT_MODULE(FCollisionCommanderModule, CollisionCommander)

#undef LOCTEXT_NAMESPACE
