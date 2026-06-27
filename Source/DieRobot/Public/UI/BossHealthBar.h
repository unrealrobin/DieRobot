// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHealthBar.generated.h"

class ABossBase;

/**
 * 
 */
UCLASS()
class DIEROBOT_API UBossHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	FName BossDisplayName = "SET_BOSS_NAME";

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI)
	ABossBase* BossActor = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	float CurrentHealth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	float MaxHealth = 0.f;
	
	
};
