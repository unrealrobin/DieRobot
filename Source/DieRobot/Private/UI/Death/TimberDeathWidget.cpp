// Property of Paracosm Industries. Dont use my shit.


#include "UI/Death/TimberDeathWidget.h"

#include "GameModes/TimberGameModeBase.h"
#include "Subsystems/Wave/WaveGameInstanceSubsystem.h"

void UTimberDeathWidget::SetLastCompletedWave(int CurrentWaveNumber)
{
	//UE_LOG(LogTemp, Warning, TEXT("Death Widget Wave Number Updated: %d"), CurrentWaveNumber);
	LastCompletedWave = CurrentWaveNumber;
	SetLastCompletedWaveText();
}

void UTimberDeathWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWaveGameInstanceSubsystem* WaveSubsystem = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
	if (WaveSubsystem)
	{
		WaveSubsystem->CurrentWaveHandle.AddDynamic(this, &UTimberDeathWidget::SetLastCompletedWave);
		//UE_LOG(LogTemp, Warning, TEXT("Death Widget Bound to Wave Subsystem."));
	}
}

void UTimberDeathWidget::ResetDeathReason()
{
	//Used on any button press on the Death Widget
	DeathReason = EDeathReason::Default;
}
