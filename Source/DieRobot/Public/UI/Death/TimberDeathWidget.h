// Property of Paracosm Industries. Dont use my shit.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimberDeathWidget.generated.h"

UENUM(BlueprintType) // Allows usage in Blueprints
enum class EDeathReason : uint8
{
	Default UMETA(DisplayName = "Default"), // Enum value with display name
	SeedaDestroyed UMETA(DisplayName = "Seeda Destroyed"),
	KipDestroyed UMETA(DisplayName = "KipDestroyed")
};

/**
 * 
 */
UCLASS()
class DIEROBOT_API UTimberDeathWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	//When the Wave System Updates the Current Wave Number, this Updates the Last Completed Wave Number.
	UFUNCTION()
	void SetLastCompletedWave(int CurrentWaveNumber);

	/* Implemented in Death UI Widget BP Graph - Called in SetLastCompletedWave()*/
	UFUNCTION(BlueprintImplementableEvent)
	void SetLastCompletedWaveText();
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Death Stats")
	int LastCompletedWave = 0;
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateDeathReasonText(EDeathReason DeathReasonText);

	UFUNCTION(BlueprintCallable)
	void ResetDeathReason();

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Death Stats" )
	EDeathReason DeathReason = EDeathReason::Default;
};
