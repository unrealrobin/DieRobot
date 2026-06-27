// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LeaderboardWidget.generated.h"

class ULeaderboard;
class ULeaderboardUserRankWidget;
class UVerticalBox;
/**
 * 
 */
UCLASS()
class DIEROBOT_API ULeaderboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	TSubclassOf<ULeaderboardUserRankWidget> UserRankWidgetClass;

	UFUNCTION()
	void ProduceTopTenLeaderboardEntries();

	UFUNCTION()
	void ProduceLocalOnlineUserRankWidget();


	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VerticalEntriesContainer;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* LocalOnlineUserVerticalBox;
	
private:
	UPROPERTY()
	ULeaderboard* LeaderboardSubsystem = nullptr;
};
