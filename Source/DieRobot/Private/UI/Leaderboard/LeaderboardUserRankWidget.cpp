// Property of Paracosm.


#include "UI/Leaderboard/LeaderboardUserRankWidget.h"

#include "Components/TextBlock.h"

void ULeaderboardUserRankWidget::SetupLeaderBoardEntry()
{
	if (RankBox && DisplayNameBox && HighestCompletedWaveBox)
	{
		RankBox->SetText(FText::AsNumber(UserRank));
		DisplayNameBox->SetText(FText::FromString(UserDisplayName));
		HighestCompletedWaveBox->SetText(FText::AsNumber(HighestWaveCompleted));
		UE_LOG(LogTemp, Warning, TEXT("Rank: %d, DisplayName: %s, HighestWaveCompleted: %lld"), UserRank, *UserDisplayName, HighestWaveCompleted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RankBox, DisplayNameBox or HighestCompletedWaveBox is not valid."));
	}
}
