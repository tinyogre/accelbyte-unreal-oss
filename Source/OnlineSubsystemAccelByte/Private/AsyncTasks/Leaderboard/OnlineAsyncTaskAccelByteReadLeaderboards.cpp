// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReadLeaderboards.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineLeaderboardSystemAccelByte"

FOnlineAsyncTaskAccelByteReadLeaderboards::FOnlineAsyncTaskAccelByteReadLeaderboards(FOnlineSubsystemAccelByte* const InABInterface,
	int32 InLocalUserNum,
	const TArray<FUniqueNetIdRef>& InUsers, 
	const FOnlineLeaderboardReadRef& InReadObject)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum, true)
	, AccelByteUsers(InUsers)
	, LeaderboardObject(InReadObject)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct FOnlineAsyncTaskAccelByteReadLeaderboards"));

	LeaderboardObject->ReadState = EOnlineAsyncTaskState::InProgress;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorMessage = TEXT("request-failed-read-leaderboards-error-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to read leaderboards, identity interface is invalid!"));
		return;
	}

	ELoginStatus::Type LoginStatus;
	LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);

	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorMessage = TEXT("request-failed-read-leaderboards-error-user-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to read leaderboards, user not logged in!"));
		return;
	}

	// Iterate Every 20 Friends and Each Iteration AccelByteUsers Decrase
	while (AccelByteUsers.Num() > 0)
	{
		IDsToProcess = FMath::Min(LeaderboardUserIdsLimit, AccelByteUsers.Num());

		FriendsUserIds.Empty();
		CurrentProcessedUsers.Empty();

		OnReadLeaderboardsSuccessHandler = TDelegateUtils<THandler<FAccelByteModelsBulkUserRankingDataV3>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadLeaderboards::OnReadLeaderboardsSuccess);
		OnReadLeaderboardsFailedHandler = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadLeaderboards::OnReadLeaderboardsFailed);

		// Take the user ids, with the maximum number of 20
		for (int32 i = 0; i < IDsToProcess; i++)
		{
			TSharedRef<const FUniqueNetIdAccelByteUser> ABUser = FUniqueNetIdAccelByteUser::CastChecked(AccelByteUsers[i]);

			FriendsUserIds.Add(ABUser->GetAccelByteId());
			CurrentProcessedUsers.Add(ABUser->GetAccelByteId(), ABUser);
		}

		// Call the endpoint using the stored user ids.
		if (!IsRunningDedicatedServer())
		{
			ApiClient->Leaderboard.GetBulkUserRankingV3(FriendsUserIds, LeaderboardObject->LeaderboardName.ToString(), OnReadLeaderboardsSuccessHandler, OnReadLeaderboardsFailedHandler);
			CountRequests++;
		}
		else
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to read leaderboards as this endpoint is not yet supported for dedicated server!"));
			CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		}

		AccelByteUsers.RemoveAt(0, IDsToProcess);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalize"));

	LeaderboardObject->ReadState = bWasSuccessful ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineLeaderboardAccelBytePtr LeaderboardsInterface = StaticCastSharedPtr<FOnlineLeaderboardAccelByte>(Subsystem->GetLeaderboardsInterface());
	if (!ensure(LeaderboardsInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates to read leaderboards as our leaderboards interface is invalid!"));
		return;
	}

	LeaderboardsInterface->TriggerOnLeaderboardReadCompleteDelegates(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::Tick()
{
	Super::Tick();

	if (CountRequests == 0)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::OnReadLeaderboardsSuccess(FAccelByteModelsBulkUserRankingDataV3 const& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Read Leaderboards Success"));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	const FOnlineLeaderboardAccelBytePtr LeaderboardsInterface = StaticCastSharedPtr<FOnlineLeaderboardAccelByte>(Subsystem->GetLeaderboardsInterface());
	if (!ensure(LeaderboardsInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to save leaderboard data as our leaderboards interface is invalid!"));
		return;
	}

	for (const auto& BulkLeaderboardResult : Result.Data)
	{
		FUniqueNetIdPtr CurrentUserId;

		// Check if the returned user ids match with the request since we support partial request
		if (CurrentProcessedUsers.Contains(BulkLeaderboardResult.UserId))
		{
			CurrentUserId = *CurrentProcessedUsers.Find(BulkLeaderboardResult.UserId);
		}
		else
		{
			continue;
		}

		// Get the user account
		TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(*CurrentUserId);

		// Put the leaderboard value into read leaderboard object reference (row data)
		FOnlineStatsRow* LeaderboardRow = LeaderboardObject.Get().FindPlayerRecord(*UserAccount->GetUserId());
		if (LeaderboardRow == NULL)
		{
			LeaderboardRow = new (LeaderboardObject->Rows)
				FOnlineStatsRow(
					UserAccount->GetDisplayName(),
					FUniqueNetIdAccelByteUser::CastChecked(IdentityInterface->GetUniquePlayerId(LocalUserNum).ToSharedRef()));
		}

		// Leaderboard all time
		LeaderboardRow->Rank = BulkLeaderboardResult.AllTime.Rank;
		LeaderboardRow->Columns.Add(FName("AllTime_Point"), BulkLeaderboardResult.AllTime.Point);

		// Put the leaderboard value into read leaderboard object reference (column meta data)
		for (const auto& ColumnMeta : LeaderboardObject->ColumnMetadata)
		{
			FVariantData* LastColumn = NULL;
			switch (ColumnMeta.DataType)
			{
				case EOnlineKeyValuePairDataType::Float:
				{
					float Value = BulkLeaderboardResult.AllTime.Point;
					LastColumn = &(LeaderboardRow->Columns.Add(ColumnMeta.ColumnName, FVariantData(Value)));
					bWasSuccessful = true;
					break;
				}

				default:
				{
					UE_LOG_ONLINE(Warning, TEXT("Unsupported key value pair during data retrieval %s"), *ColumnMeta.ColumnName.ToString());
					break;
				}
			}
		}
	}

	if (CountRequests > 0)
	{
		CountRequests--;
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadLeaderboards::OnReadLeaderboardsFailed(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorCode = FString::Printf(TEXT("%d"), Code);
	ErrorMessage = ErrMsg;

	CountRequests--;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE