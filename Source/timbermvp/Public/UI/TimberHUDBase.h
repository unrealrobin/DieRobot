// Property of Paracosm Industries.

#pragma once

#include "Controller/TimberPlayerController.h"
#include "Delegates/Delegate.h"
#include "Delegates/DelegateCombinations.h"
#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TimberHUDBase.generated.h"


class UBossHealthBar;
/**
 * 
 */
UCLASS()
class TIMBERMVP_API ATimberHUDBase : public AHUD
{
	GENERATED_BODY()

public:
	
	//void SeedaBinding();
	virtual void BeginPlay() override;
	void CreateWelcomeMessageWidget();

	void InitializeWidgets();
	
	UFUNCTION()
	void UpdateDeathUIReason_KipDestroyed(bool bIsPlayerDead);

	UFUNCTION()
	void ToggleSettingsPanelWidget();
	
	void CharacterAndControllerBindings();
	void GameModeBindings();
	void BindToWaveSubsystem();
	
	UFUNCTION()
	void UpdateDeathUIReason_SeedaDestroyed(bool bIsSeedaDestroyed);
	
	/* Delegates */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIsBuildMenuOpen, bool, bIsBuildMenuOpen);
	UPROPERTY()
	FIsBuildMenuOpen bIsBuildMenuOpen;

	/*
	 * Widget Classes
	 * Set in BP on BP_TimberHUDBase
	 */
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> RootWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> BuildMenuWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> DeathWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> DeleteBuildingComponentWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> SeedaOverlapWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> AmmoCounterWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> KBM_MovementControlsWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> KBM_CombatControlsWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> KBM_BuildControlWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> BossHealthBarWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> SettingsPanelWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UUserWidget> WelcomeWidgetClass;
	
	//Widget References
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* RootWidget;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* BuildMenuWidget;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* DeathWidget;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* DeleteBuildingComponentWidget;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* SeedaOverlapWidget;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* AmmoCounterWidget;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* KBM_MovementControlsWidget;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* KBM_CombatControlsWidget;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* KBM_BuildControlsWidget;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* BossHealthBarWidget;
	UPROPERTY(VisibleAnywhere)
	UUserWidget* SettingsPanelWidget;
	UPROPERTY(BlueprintReadOnly)
	UUserWidget* WelcomeWidget;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ATimberPlayerController> TimberPlayerController;
	
	UFUNCTION(BlueprintCallable, Category = "Build Menu")
	void OpenBuildPanelMenu();
	
	UFUNCTION(BlueprintCallable, Category = "Build Menu")
	void CloseBuildPanelMenu();
	
	UFUNCTION(BlueprintCallable, Category="Death UI")
	void SwitchToDeathUI();
	
	UFUNCTION(BlueprintCallable)
	void SwitchToGameUI();
	
	UFUNCTION(Category="Action UI")
	void ShowDeleteBuildingComponentWidget(float ViewportLocationX, float ViewportLocationY);
	
	UFUNCTION()
	void HideDeleteBuildingComponentWidget();

	UFUNCTION()
	void HandleAmmoCounterVisibility(bool bShouldShowAmmoCounter);
	
	FVector2d GetCenterOfScreen();
	FVector2d GetViewportSize();

	UFUNCTION()
	void ShowAllGameWidgets();

	UFUNCTION()
	void ToggleBuildMenuStatusEffectDetails();

protected:
	UPROPERTY()
	ATimberPlayableCharacter* TimberCharacter;

	UFUNCTION()
	void SetWidgetToFocus(UUserWidget* Widget);

	UFUNCTION()
	void SetGameToFocus();
	
	/*Delegate Listeners*/
	UFUNCTION()
	void HandleBuildPanelMenu(bool IsBuildPanelMenuOpen);

	//Vars to shift in the X & Y direction when the DeleteBuildingComponentWidget is spawned. Shifted from Center.
	FVector2d DeleteWidgetLocation = FVector2d(0, 0);
	float DeleteBuildingComponentWidgetShiftX = 100.f;
	float DeleteBuildingComponentWidgetShiftY = 100.f;

private:
	/* Tutorial State Handling*/
	UFUNCTION()
	void InitializeTutorialStateBinding();

	UFUNCTION()
	ETutorialState GetTutorialState();

	UFUNCTION()
	void HandleTutorialStateChanges(ETutorialState NewState);

	UPROPERTY()
	TArray<UUserWidget*> RootWidgetChildrenWidgets;

	void GetRootWidgetChildrenWidgets();

	UUserWidget* GetWidgetByClassName(FString ClassName);
	UUserWidget* CreateVisibleWidget(const TSubclassOf<UUserWidget>& Class, int32 ZOrder);
	UUserWidget* CreateHiddenWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder);

	//TODO: Tutorial Needs Refinement
	void HideWidget(UUserWidget* Widget);
	void ShowWidget(UUserWidget* Widget);
	void HideAllChildWidgets(TArray<UUserWidget*> Widgets);
	void ShowCrossHairWidget();
	void ShowInventoryPanelWidget();
	void ShowPlayerHealthWidget();
	void ShowSeedaHealthWidget();
	void ShowWaveDataWidget();
	void SeedaBindings();

	UFUNCTION()
	void HandleBossDeath();
	UFUNCTION()
	void HandleBossSpawned(AActor* BossActor);

	
	
};









