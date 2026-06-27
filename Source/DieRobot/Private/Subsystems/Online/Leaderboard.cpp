// Property of Paracosm.


#include "Subsystems/Online/Leaderboard.h"
#include "Online/Leaderboards.h"
#include "Online/OnlineAsyncOpHandle.h"
#include "Online/OnlineResult.h"
#include "Online/Stats.h"
#include "Subsystems/Online/Login.h"
#include "Subsystems/Wave/WaveGameInstanceSubsystem.h"

DEFINE_LOG_CATEGORY(LogLeaderboard)

void ULeaderboard::HandleBindToWaveSubsytem()
{
	//Gets called from the WaveGameInstanceSubsystem after initialization to ensure we can bind to OnWaveComplete Delegate.
	if (UWaveGameInstanceSubsystem* WaveSubsystem = GetGameInstance()->GetSubsystem<UWaveGameInstanceSubsystem>())
	{
		WaveSubsystem->OnWaveComplete.AddDynamic(this, &ULeaderboard::OnWaveComplete);
		UE_LOG(LogLeaderboard, Warning, TEXT("Bound to Wave Subsystem. WORKING."));
	}
	else
	{
		UE_LOG(LogLeaderboard, Warning, TEXT("Not Bound to Wave Subsystem."));
		
	}
}

void ULeaderboard::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeOnlineServices();
	UE_LOG(LogLeaderboard, Warning, TEXT("Leaderboard Subsystem Initialized."));
}

ULogin* ULeaderboard::GetLoginSubsystem()
{
	if (ULogin* LoginSubsystem = GetGameInstance()->GetSubsystem<ULogin>())
	{
		return LoginSubsystem;
	}

	return nullptr;
}

void ULeaderboard::UpdateLocalOnlineUserStat(int CompletedWaveNumber)
{
	ULogin* Login = GetLoginSubsystem();
	if (StatsService.IsValid() && Login)
	{
		//Get Local Account ID - Available on Login Struct
		FAccountId PlayerAccountId = Login->LocalUserInfo.AccountInfo.AccountId;;
		
		//Configuring the StatValue
		int64 WaveNumberAsINT64 = static_cast<int64>(CompletedWaveNumber);
		FStatValue StatValue;
		StatValue.VariantType = ESchemaAttributeType::Int64;
		StatValue.Set(WaveNumberAsINT64);

		//Setting Which stat to Update with which value.
		FUserStats UserStats;
		UserStats.AccountId = PlayerAccountId;
		UserStats.Stats.Add(TEXT("HIGHEST_WAVE_COMPLETED"), StatValue);
		
		//Utilizing the UpdateStats function to send the data to the backend.
		FUpdateStats::Params Params;
		Params.LocalAccountId = PlayerAccountId;
		Params.UpdateUsersStats.Add(UserStats);
		StatsService->UpdateStats(MoveTemp(Params)).OnComplete([this](const TOnlineResult<FUpdateStats>& Result)
		{
			if (Result.IsOk())
			{
				if (GEngine)
				{
					UE_LOG(LogLeaderboard, Warning, TEXT("Sent Updated Stats to Stat Service. Result is OK."));
				}
			}
			else
			{
				UE_LOG(LogLeaderboard, Warning, TEXT("Sent Updated Stats to Stat Service. Result is NOT OK."));
			}
		});
	}
}

void ULeaderboard::OnWaveComplete(int CompletedWaveNumber)
{
	UpdateLocalOnlineUserStat(CompletedWaveNumber);
}

void ULeaderboard::InitializeOnlineServices()
{
	OnlineService = GetServices();
	if (OnlineService.IsValid())
	{
		LeaderboardService = OnlineService->GetLeaderboardsInterface();
		StatsService = OnlineService->GetStatsInterface();
		if (LeaderboardService.IsValid() && StatsService.IsValid())
		{
			UE_LOG(LogLeaderboard, Warning, TEXT("Leaderboard and Stats service is valid."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Leaderboard or Stats service is not valid."));
		}
	}
}

void ULeaderboard::QueryTopTenLeaderboard()
{
	if (OnlineService.IsValid() && LeaderboardService.IsValid())
	{

		FReadEntriesAroundRank::Params Params;
		Params.LocalAccountId = GetLoginSubsystem()->LocalUserInfo.AccountInfo.AccountId;
		Params.Rank = 0;
		Params.Limit = 10;
		Params.BoardName = TEXT("HIGHEST_WAVE_COMPLETED");

		UE_LOG(LogLeaderboard, Warning, TEXT("Leaderboard Calling Read Entries Around Rank."));
		LeaderboardService->ReadEntriesAroundRank(MoveTemp(Params)).OnComplete([this]
		(const TOnlineResult<FReadEntriesAroundRank>& Result)
		{
			if (Result.IsOk())
			{
				UE_LOG(LogLeaderboard, Warning, TEXT("Leaderboard Query IsOk."));
				TopTenEntries = Result.GetOkValue().Entries;

				if (TopTenEntries.Num() > 0)
				{
					OnSuccessfulLeaderboardQuery.Broadcast(); //Notifying Leaderboard Widget that the query was successful.
					UE_LOG(LogLeaderboard, Warning, TEXT("Broadcasting Successful Leaderboard Entries."));
				}
				
				//DEBUGGING PURPOSES
				/*for (const FLeaderboardEntry& Entry : TopTenEntries)
				{
					FAccountId AccountId = Entry.AccountId;
					FString EntryDisplayName = GetLoginSubsystem()->GetDisplayNameFromAccountId(&AccountId);
					UE_LOG(LogLeaderboard, Warning, TEXT("Rank: %d, DisplayName: %s, Score: %lld"),
						Entry.Rank, *EntryDisplayName, Entry.Score);
				}*/
			}
			else if (Result.IsError())
			{
				UE_LOG(LogLeaderboard, Warning, TEXT("Leaderboard Query Failed."));
			}
		});
	}
}

void ULeaderboard::QueryLocalUserLeaderboardRank(FString BoardName)
{
	//Retriev the Local User Account id from the Login Subsystem

	ULogin* LoginSubsystem = GetLoginSubsystem();
	if (LoginSubsystem && LeaderboardService.IsValid())
	{
		FAccountId AccountId = LoginSubsystem->LocalUserInfo.AccountInfo.AccountId;

		/*{
			FReadEntriesForUsers::Params Params;
			Params.BoardName = BoardName;
			Params.AccountIds.Add(AccountId);
			Params.LocalAccountId = AccountId;

			LeaderboardService->ReadEntriesForUsers(MoveTemp(Params)).OnComplete([this]
				(const TOnlineResult<FReadEntriesForUsers>& Result)
				{
					if (Result.IsOk())
					{
						TArray<FLeaderboardEntry> Entries = Result.GetOkValue().Entries;
						if (Entries.Num() > 0)
						{
							UE_LOG(LogTemp, Warning, TEXT("There are %d Entries for the Local User."), Entries.Num());
							//Populating a Local Struct on the Leaderboard Subsystem with the Local User Data.
							LocalOnlineUserData.LocalOnlineUserRank = Entries[0].Rank;
							LocalOnlineUserData.LocalOnlineUserScore = Entries[0].Score;
							LocalOnlineUserData.DisplayName = GetLoginSubsystem()->GetDisplayNameFromAccountId(&Entries[0].AccountId);
							UE_LOG(LogTemp, Warning, TEXT("DisplayName: %s, Rank: %d, Score: %lld"),*LocalOnlineUserData.DisplayName, LocalOnlineUserData.LocalOnlineUserRank, LocalOnlineUserData.LocalOnlineUserScore);
							OnSuccessfulLeaderboardQuery.Broadcast();

							if (Entries.Num() > 1)
							{
								for (FLeaderboardEntry Entry : Entries)
								{
									FString EntryDisplayName = GetLoginSubsystem()->GetDisplayNameFromAccountId(&Entry.AccountId);
									UE_LOG(LogTemp, Warning, TEXT("Rank: %d, DisplayName: %s, Score: %lld"),
										Entry.Rank, *EntryDisplayName, Entry.Score);
								}
							

							}
						}
					}
					else if (Result.IsError())
					{
						UE_LOG(LogLeaderboard, Warning, TEXT("Local Online user is Unranked or other Error."));
					}
				});
		}*/

		FReadEntriesAroundUser::Params Params;
		Params.Limit = 1;
		Params.Offset = 0;
		Params.AccountId = AccountId;
		Params.LocalAccountId = AccountId;
		Params.BoardName = TEXT("HIGHEST_WAVE_COMPLETED");
		LeaderboardService->ReadEntriesAroundUser(MoveTemp(Params)).OnComplete([this]
		(const TOnlineResult<FReadEntriesAroundUser>& Result)
		{
			if (Result.IsOk())
			{
				TArray<FLeaderboardEntry> Entries = Result.GetOkValue().Entries;
				if (Entries.Num() > 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("There are %d Entries for the Local User."), Entries.Num());
					//Populating a Local Struct on the Leaderboard Subsystem with the Local User Data.
					LocalOnlineUserData.LocalOnlineUserRank = Entries[0].Rank;
					LocalOnlineUserData.LocalOnlineUserScore = Entries[0].Score;
					LocalOnlineUserData.DisplayName = GetLoginSubsystem()->GetDisplayNameFromAccountId(&Entries[0].AccountId);
					UE_LOG(LogTemp, Warning, TEXT("DisplayName: %s, Rank: %d, Score: %lld"),*LocalOnlineUserData.DisplayName, LocalOnlineUserData.LocalOnlineUserRank, LocalOnlineUserData.LocalOnlineUserScore);

					//Broadcast to Update UI
					OnSuccessfulLeaderboardQuery.Broadcast();
				}
				else
				{
					UE_LOG(LogLeaderboard, Warning, TEXT("No Entries for the Local User."));
				}
			}
		});
	}
}


