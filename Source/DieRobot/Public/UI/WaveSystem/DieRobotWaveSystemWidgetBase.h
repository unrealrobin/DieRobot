// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DieRobotWaveSystemWidgetBase.generated.h"


class ADieRobotGameModeBase;
/**
 * 
 */
UCLASS()
class DIEROBOT_API UDieRobotWaveSystemWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Wave Data")
	float CurrentWave = 1;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Wave Data")
	int TimeToNextWaveWidget = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ADieRobotGameModeBase* GameMode;

protected:
	UFUNCTION()
	void UpdateCurrentWave(int CurrentWaveNumber_FromSubsystem);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdatedWaveNumberUI(float CurrentWaveNumber);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTimeToNextWaveSeconds(int Seconds);
};
