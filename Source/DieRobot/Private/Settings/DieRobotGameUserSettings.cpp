// Property of Paracosm.


#include "Settings/DieRobotGameUserSettings.h"

void UDieRobotGameUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{

	SetScreenResolution(FIntPoint(1920, 1080));
	SetFullscreenMode(EWindowMode::Windowed);

	ConfirmVideoMode();
	
	Super::ApplySettings(bCheckForCommandLineOverrides);

	SaveSettings();
}
