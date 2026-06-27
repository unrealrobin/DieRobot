// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmmoCounter.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UAmmoCounter : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int CurrentAmmo = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int MaxAmmo = 0.f;
};
