// Property of Paracosm Industries. Dont use my shit.


#include "UI/WaveSystem/TimberWaveSystemWidgetBase.h"

#include "GameModes/TimberGameModeBase.h"
#include "Subsystems/Wave/WaveGameInstanceSubsystem.h"

void UTimberWaveSystemWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	if(!GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>()->CurrentWaveHandle.IsBound())
	{
		GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>()->CurrentWaveHandle.AddDynamic(this, &UTimberWaveSystemWidgetBase::UpdateCurrentWave);
	}
	if(!GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>()->TimeToNextWaveSecondsHandle.IsBound())
	{
		GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>()->TimeToNextWaveSecondsHandle.AddDynamic(this, &UTimberWaveSystemWidgetBase::UpdateTimeToNextWaveSeconds);
	}
	
}

void UTimberWaveSystemWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Widget is always checking if a timer is going to the wave timer. if there is less than 15 seconds it updates the time on the widget.
	// TODO:: Change this to a delegate call from the Wave Subsystem to update the time.
	if (GetWorld() && GetGameInstance())
	{
		UWaveGameInstanceSubsystem* WaveSubsystem = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>();
		if (WaveSubsystem && GetWorld()->GetTimerManager().IsTimerActive(WaveSubsystem->TimeToNextWaveHandle))
		{
			int TempTime = FMath::CeilToInt(GetWorld()->GetTimerManager().GetTimerRemaining(WaveSubsystem->TimeToNextWaveHandle));
			if (TempTime)
			{
				//Reduced unchanging updates to the widget
				if (TempTime != TimeToNextWaveWidget)
				{
					TimeToNextWaveWidget = TempTime;
				}
			}
			else
			{
				if(TimeToNextWaveWidget != 0)
				{
					TimeToNextWaveWidget = 0;
				}
			}
		}
	}

}

void UTimberWaveSystemWidgetBase::UpdateCurrentWave(int CurrentWaveNumber_FromSubsystem)
{
	CurrentWave = CurrentWaveNumber_FromSubsystem;
	UpdatedWaveNumberUI(CurrentWave);
}
