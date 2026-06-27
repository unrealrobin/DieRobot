// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "LeaderboardUserRankWidget.generated.h"

/**
 * 
 */
UCLASS()
class DIEROBOT_API ULeaderboardUserRankWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RankBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DisplayNameBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HighestCompletedWaveBox;

	UFUNCTION()
	void SetupLeaderBoardEntry();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString UserDisplayName = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 UserRank = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int64 HighestWaveCompleted = 0;
};
