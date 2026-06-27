// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildingComponent.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UBuildingComponent : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FSlateBrush BuildingComponentIconImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Buildable")
	UDataAsset* BuildingComponentDataAsset;
};
