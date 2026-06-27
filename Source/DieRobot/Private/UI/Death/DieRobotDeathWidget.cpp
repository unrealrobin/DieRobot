// Property of Paracosm Industries. Dont use my shit.


#include "UI/Death/DieRobotDeathWidget.h"

#include "GameModes/DieRobotGameModeBase.h"
#include "Subsystems/Wave/WaveGameInstanceSubsystem.h"

void UDieRobotDeathWidget::SetLastCompletedWave(int CurrentWaveNumber)
{
	//UE_LOG(LogTemp, Warning, TEXT("Death Widget Wave Number Updated: %d"), CurrentWaveNumber);
	LastCompletedWave = CurrentWaveNumber;
	SetLastCompletedWaveText();
}

void UDieRobotDeathWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWaveGameInstanceSubsystem* WaveSubsystem = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
	if (WaveSubsystem)
	{
		WaveSubsystem->CurrentWaveHandle.AddDynamic(this, &UDieRobotDeathWidget::SetLastCompletedWave);
		//UE_LOG(LogTemp, Warning, TEXT("Death Widget Bound to Wave Subsystem."));
	}
}

void UDieRobotDeathWidget::ResetDeathReason()
{
	//Used on any button press on the Death Widget
	DeathReason = EDeathReason::Default;
}
