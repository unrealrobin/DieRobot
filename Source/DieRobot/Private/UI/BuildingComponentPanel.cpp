// Property of Paracosm Industries.


#include "UI/BuildingComponentPanel.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "BuildSystem/Traps/TrapBase.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Data/DataAssets/BuildComponentDataAsset.h"
#include "Data/DataAssets/StatusEffects/StatusEffectDefinition.h"
#include "UI/StatusEffects/EffectDefinitionItem.h"


void UBuildingComponentPanel::LoadAllDataAssetsForMenu()
{
	//TO BE CALLED FROM THE GAME MODE BEGIN PLAY
	FString PathToBuildableDataAssets = TEXT("/Game/Assets/Buildables/00_DataAssets/BuildingComponentDataAsset");
	//Loading Asset Registry Module
	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	if (AssetRegistry.IsValid())
	{
		//Scan the folder with the Data Assets / ensures the assets are registered.
		AssetRegistry.Get().ScanPathsSynchronous({PathToBuildableDataAssets});

		//Get all assets at this path and populate the AssetData (not data asset) into AssetDataArray<FAssetData>
		AssetRegistry.Get().GetAssetsByPath(FName(*PathToBuildableDataAssets), AssetDataArray, true);

		for (FAssetData& AssetData : AssetDataArray)
		{
			UObject* LoadedAsset = AssetData.GetAsset();
			if (LoadedAsset)
			{
				UDataAsset* DataAsset = Cast<UDataAsset>(LoadedAsset);
				if (DataAsset)
				{
					AllBuildablesArray.Add(DataAsset);
				}
			}
		}

		/*
		 * Each DA has some Sort Value set in BP's so the order in the UI Widget is Predetermined.
		 */
		if (AllBuildablesArray.Num() > 0)
		{
			AllBuildablesArray.Sort([](const UDataAsset& A, const UDataAsset& B )
			{
				const UBuildComponentDataAsset* AssetA = Cast<const UBuildComponentDataAsset>(&A);
				const UBuildComponentDataAsset* AssetB = Cast<const UBuildComponentDataAsset>(&B);

				if (!AssetA || !AssetB)
				{
					return false;
				}

				return AssetA->SortValue < AssetB->SortValue;
			});
		}
	}
}

void UBuildingComponentPanel::PrepareTrapEffectList()
{
	//Get the DA for the Status Effect.
	//Get the default settings on the status Effect from the Status Effect CLass.
	if (BuildMenuHoveredIconDataAsset && EffectDefinitionItemClass)
	{
		if (UBuildComponentDataAsset* BuildComponentDataAsset = Cast<UBuildComponentDataAsset>(BuildMenuHoveredIconDataAsset))
		{
			if (BuildComponentDataAsset->BuildingComponentClass && BuildComponentDataAsset->BuildingComponentClass->IsChildOf(ATrapBase::StaticClass()))
			{
				ATrapBase* TrapBase = BuildComponentDataAsset->BuildingComponentClass->GetDefaultObject<ATrapBase>();
				//Loop through the Status Effect Definitions.
				//For Each Definition
				for (UStatusEffectDefinition* EffectDefinition: TrapBase->StatusEffectDefinitions)
				{
					//Create an Effect Definition Info Widget
					UEffectDefinitionItem* EffectDefinitionWidget = CreateWidget<UEffectDefinitionItem>(this, EffectDefinitionItemClass);
					if (EffectDefinitionWidget)
					{
						EffectDefinitionWidget->EffectDefinition = EffectDefinition;
						EffectDefinitionWidget->TrapDefaultObject = TrapBase;
						EffectDefinitionWidget->ConfigureDataDisplay();
						TrapDetailsVerticalBox->AddChild(EffectDefinitionWidget);
					}
				}
			}
		}
	}
}

void UBuildingComponentPanel::RemoveTrapEffectList()
{
	if (TrapDetailsVerticalBox->GetChildrenCount() > 0)
	{
		TArray<UWidget*> ChildrenWidgets = TrapDetailsVerticalBox->GetAllChildren();
		for (UWidget* Child : ChildrenWidgets)
		{
			if (Child->IsA(UEffectDefinitionItem::StaticClass()))
			{
				TrapDetailsVerticalBox->RemoveChild(Child);
			}
		}
		ChildrenWidgets.Empty();
	}
}

void UBuildingComponentPanel::SetHasStatusEffect(bool bNewHasStatusEffect)
{
	bHasStatusEffect = bNewHasStatusEffect;
}

void UBuildingComponentPanel::NativeConstruct()
{
	Super::NativeConstruct();
	//Getting all the Data Assets for the Building Components
	LoadAllDataAssetsForMenu();
	//Creating the Icons in the Menu
	CreateBuildableIconsInMenu();
}
