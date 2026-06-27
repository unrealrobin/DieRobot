// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FloatingDataWidget.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API UFloatingDataWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage Number")
	bool bIsDamage = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Text")
	FName EffectName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Number")
	float DamageNumber = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Number")
	float DamageSize = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Number")
	FLinearColor DamageColor = FLinearColor::White;
};
