// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildingComponentPanel.generated.h"

class UEffectDefinitionItem;
class UVerticalBox;
class FAssetRegistryModule;
class ATrapBase;
class UTextBlock;

UENUM(BlueprintType)
enum class EActiveTab: uint8
{
	StructureTab UMETA(DisplayName = "Structure"),
	TrapTab UMETA(DisplayName = "Trap"),
	ConstructTab UMETA(DisplayName = "Construct")
};


/**
 * 
 */
UCLASS()
class DIEROBOT_API UBuildingComponentPanel : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	void ToggleStatusEffectDetails();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Tabs Status")
	EActiveTab ActiveTab = EActiveTab::StructureTab;

protected:

	UFUNCTION()
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void LoadAllDataAssetsForMenu();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void CreateBuildableIconsInMenu();

	UFUNCTION(BlueprintCallable)
	void PrepareTrapEffectList();

	UFUNCTION(BlueprintCallable)
	void RemoveTrapEffectList();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Menu")
	TSubclassOf<UEffectDefinitionItem> EffectDefinitionItemClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Build Menu")
	UDataAsset* BuildMenuHoveredIconDataAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Build Menu")
	bool bIsIconFocusedOrHovered = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buildables")
	TArray<UDataAsset*> AllBuildablesArray;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FAssetData> AssetDataArray;

	/*Status Effect Vars*/
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status Effects")
	bool bHasStatusEffect = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status Effects")
	float InitialDamage = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status Effects")
	float DamagePerTick = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status Effects")
	float EffectDuration = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status Effects")
	int MaxStacks = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status Effects")
	bool bSlows = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status Effects")
	bool bRemovesSlow = false;

	UFUNCTION(BlueprintCallable)
	void SetHasStatusEffect(bool bNewHasStatusEffect);
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* TrapDetailsVerticalBox;
	
};
