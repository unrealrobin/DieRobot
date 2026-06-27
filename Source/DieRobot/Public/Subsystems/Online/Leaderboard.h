// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "Online/Leaderboards.h"
#include "Online/OnlineServices.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Leaderboard.generated.h"


class ULogin;
using namespace UE::Online;

USTRUCT(BlueprintType)
struct FLocalOnlineUserLeaderboardRank
{
	GENERATED_BODY()

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 LocalOnlineUserRank = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int64 LocalOnlineUserScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString DisplayName = TEXT("Unranked");
	
};

/**
 * 
 */

DECLARE_LOG_CATEGORY_EXTERN(LogLeaderboard, Log, All);

UCLASS()
class DIEROBOT_API ULeaderboard : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSuccessfulLeaderboardQuery);
	FSuccessfulLeaderboardQuery OnSuccessfulLeaderboardQuery;
	
	void HandleBindToWaveSubsytem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void InitializeOnlineServices();
	
	void QueryLocalUserLeaderboardRank(FString BoardName);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FLocalOnlineUserLeaderboardRank LocalOnlineUserData;

	void QueryTopTenLeaderboard();

	//Populated by QueryTopTenLeaderboard()
	TArray<FLeaderboardEntry> TopTenEntries;
	
	ULogin* GetLoginSubsystem();
private:
	IOnlineServicesPtr OnlineService = nullptr;
	ILeaderboardsPtr LeaderboardService = nullptr;
	IStatsPtr StatsService = nullptr;
	IAuthPtr LoginService = nullptr;

	
	void UpdateLocalOnlineUserStat(int CompletedWaveNumber);

	/*Wave Information*/
	UFUNCTION()
	void OnWaveComplete(int CompletedWaveNumber);
};
