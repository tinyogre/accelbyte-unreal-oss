// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Models/AccelByteSessionModels.h"
#include "Models/AccelByteMatchmakingModels.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteMatchmakingModels.h"

class FInternetAddr;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionInfoAccelByteV2 : public FOnlineSessionInfo
{
public:
	FOnlineSessionInfoAccelByteV2(const FString& SessionIdStr);

	//~ Begin FOnlineSessionInfo overrides
	const FUniqueNetId& GetSessionId() const override;
	const uint8* GetBytes() const override;
	int32 GetSize() const override;
	bool IsValid() const override;
	FString ToString() const override;
	FString ToDebugString() const override;
	//~ End FOnlineSessionInfo overrides

	/** Set the host address for this session if one is available, used for game sessions */
	void SetHostAddress(const TSharedRef<FInternetAddr>& InHostAddress);

	/** Get the host address for this session, may be nullptr */
	TSharedPtr<FInternetAddr> GetHostAddress() const;

	/** Set the current backend session data associated with this session */
	void SetBackendSessionData(const TSharedPtr<FAccelByteModelsV2BaseSession>& InBackendSessionData);

	/** Get the locally stored backend session data for this session */
	TSharedPtr<FAccelByteModelsV2BaseSession> GetBackendSessionData() const;

	/** Get the locally stored team assignments for this session */
	TArray<FAccelByteModelsV2GameSessionTeam> GetTeamAssignments() const;

	/** Set the locally stored team assignments for this session, can be updated on backend with UpdateSession */
	void SetTeamAssignments(const TArray<FAccelByteModelsV2GameSessionTeam>& InTeams);

	/** Whether or not this session info structure has information to allow the player to connect to it remotely */
	bool HasConnectionInfo() const;

	/** Return connection info that this session has as a string, or blank if none is found */
	FString GetConnectionString() const;

	/** Returns the ID of the leader of this session. Only valid for party sessions, will be nullptr otherwise. */
	FUniqueNetIdPtr GetLeaderId() const;

PACKAGE_SCOPE:
	/**
	 * Update the list of invited players on this session from the backend session data.
	 */
	void UpdateInvitedPlayers();

	/**
	 * Update the stored leader ID for this session. Intended only for use with party sessions.
	 */
	void UpdateLeaderId();

	/**
	 * Update the stored connection information for this server.
	 */
	void UpdateConnectionInfo();

private:
	/**
	 * Structure representing the session data on the backend, used for updating session data.
	 */
	TSharedPtr<FAccelByteModelsV2BaseSession> BackendSessionData{nullptr};

	/**
	 * ID of the session that this information is for
	 */
	TSharedRef<const FUniqueNetIdAccelByteResource> SessionId;

	/**
	 * IP address for this session
	 */
	TSharedPtr<FInternetAddr> HostAddress{nullptr};

	/**
	 * Team assignments for this session, only used for game session
	 */
	TArray<FAccelByteModelsV2GameSessionTeam> Teams{};

	/**
	 * Players that have been invited to this session
	 */
	TArray<FUniqueNetIdRef> InvitedPlayers{};

	/**
	 * ID of the leader of this session. Only will be valid for party sessions.
	 */
	FUniqueNetIdPtr LeaderId{};

};

UENUM(BlueprintType)
enum class EOnlineSessionTypeAccelByte : uint8
{
	Unknown = 0,
	GameSession,
	PartySession
};

/**
 * Structure representing a session that was restored through Session::GetMyGameSessions or Session::GetMyParties
 */
struct ONLINESUBSYSTEMACCELBYTE_API FOnlineRestoredSessionAccelByte
{
	FOnlineRestoredSessionAccelByte()
	{
	}

	FOnlineRestoredSessionAccelByte(const EOnlineSessionTypeAccelByte& InSessionType, const FOnlineSessionSearchResult& InSession)
		: SessionType(InSessionType)
		, Session(InSession)
	{
	}

	/** Session type of the restored session */
	EOnlineSessionTypeAccelByte SessionType{EOnlineSessionTypeAccelByte::Unknown};

	/** Search result of the restored session */
	FOnlineSessionSearchResult Session{};
};

/**
 * Structure representing a session invite
 */
struct ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionInviteAccelByte
{
	/** Type of session that this invite is for */
	EOnlineSessionTypeAccelByte SessionType{EOnlineSessionTypeAccelByte::Unknown};

	/** ID of the user that sent this invite, could be nullptr */
	FUniqueNetIdPtr SenderId{nullptr};

	/** Search result containing this session for join */
	FOnlineSessionSearchResult Session{};
};

/**
 * AccelByte specific subclass for an online session search handle. Stores ticket ID and matchmaking user ID for retrieval later.
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionSearchAccelByte : public FOnlineSessionSearch
{
public:
	FOnlineSessionSearchAccelByte() = default;
	FOnlineSessionSearchAccelByte(const TSharedRef<FOnlineSessionSearch>& InBaseSearch);

	FUniqueNetIdPtr GetSearchingPlayerId() const;
	FString GetTicketId() const;
	FName GetSearchingSessionName() const;

PACKAGE_SCOPE:
	/**
	 * ID of the player that is currently searching for a match with this handle.
	 */
	FUniqueNetIdPtr SearchingPlayerId{nullptr};

	/**
	 * ID of the ticket a player is apart of for the search
	 */
	FString TicketId{};

	/**
	 * Name of the local session that we intend to matchmake for
	 */
	FName SearchingSessionName{};

};

/**
 * Convenience method to convert a single user ID for a start matchmaking request into a array of matchmaking users,
 * containing the single user.
 */
#define USER_ID_TO_MATCHMAKING_USER_ARRAY(UserId) TArray<FSessionMatchmakingUser>{{UserId}}

// Begin custom delegates
DECLARE_DELEGATE_TwoParams(FOnRestorePartySessionsComplete, const FUniqueNetId& /*LocalUserId*/, const FOnlineError& /*Result*/);
DECLARE_DELEGATE_TwoParams(FOnRestoreActiveSessionsComplete, const FUniqueNetId& /*LocalUserId*/, const FOnlineError& /*Result*/);
DECLARE_DELEGATE_OneParam(FOnRegisterServerComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_OneParam(FOnUnregisterServerComplete, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_TwoParams(FOnLeaveSessionComplete, bool /*bWasSuccessful*/, FString /*SessionId*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGetServerAssociatedSessionComplete, FName /*SessionName*/);
typedef FOnGetServerAssociatedSessionComplete::FDelegate FOnGetServerAssociatedSessionCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQueryAllInvitesComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*PlayerId*/);
typedef FOnQueryAllInvitesComplete::FDelegate FOnQueryAllInvitesCompleteDelegate;

DECLARE_MULTICAST_DELEGATE(FOnInviteListUpdated);
typedef FOnInviteListUpdated::FDelegate FOnInviteListUpdatedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnV2SessionInviteReceived, const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FromId*/, const FOnlineSessionInviteAccelByte& /*Invite*/);
typedef FOnV2SessionInviteReceived::FDelegate FOnV2SessionInviteReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionServerUpdate, FName /*SessionName*/);
typedef FOnSessionServerUpdate::FDelegate FOnSessionServerUpdateDelegate;

DECLARE_MULTICAST_DELEGATE(FOnMatchmakingStarted)
typedef  FOnMatchmakingStarted::FDelegate FOnMatchmakingStartedDelegate;
//~ End custom delegates

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionV2AccelByte : public IOnlineSession, public TSharedFromThis<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe>
{
public:
	virtual ~FOnlineSessionV2AccelByte() {}

	// Begin IOnlineSession overrides
	TSharedPtr<const FUniqueNetId> CreateSessionIdFromString(const FString& SessionIdStr) override;
	class FNamedOnlineSession* GetNamedSession(FName SessionName) override;
	void RemoveNamedSession(FName SessionName) override;
	bool HasPresenceSession() override;
	EOnlineSessionState::Type GetSessionState(FName SessionName) const override;
	bool CreateSession(int32 HostingPlayerNum, FName SessionName, const FOnlineSessionSettings& NewSessionSettings) override;
	bool CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings) override;
	bool StartSession(FName SessionName) override;
	bool UpdateSession(FName SessionName, FOnlineSessionSettings& UpdatedSessionSettings, bool bShouldRefreshOnlineData = true) override;
	bool EndSession(FName SessionName) override;
	bool DestroySession(FName SessionName, const FOnDestroySessionCompleteDelegate& CompletionDelegate = FOnDestroySessionCompleteDelegate()) override;
	bool IsPlayerInSession(FName SessionName, const FUniqueNetId& UniqueId) override;
	bool StartMatchmaking(const TArray<TSharedRef<const FUniqueNetId>>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
	bool StartMatchmaking(const TArray<FSessionMatchmakingUser>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings, const FOnStartMatchmakingComplete& CompletionDelegate) override;
	bool CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName) override;
	bool CancelMatchmaking(const FUniqueNetId& SearchingPlayerId, FName SessionName) override;
	bool FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
	bool FindSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings) override;
	bool FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate) override;
	bool CancelFindSessions() override;
	bool PingSearchResults(const FOnlineSessionSearchResult& SearchResult) override;
	bool JoinSession(int32 LocalUserNum, FName SessionName, const FOnlineSessionSearchResult& DesiredSession) override;
	bool JoinSession(const FUniqueNetId& LocalUserId, FName SessionName, const FOnlineSessionSearchResult& DesiredSession) override;
	bool FindFriendSession(int32 LocalUserNum, const FUniqueNetId& Friend) override;
	bool FindFriendSession(const FUniqueNetId& LocalUserId, const FUniqueNetId& Friend) override;
	bool FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<TSharedRef<const FUniqueNetId>>& FriendList) override;
	bool SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Friend) override;
	bool SendSessionInviteToFriend(const FUniqueNetId& LocalUserId, FName SessionName, const FUniqueNetId& Friend) override;
	bool SendSessionInviteToFriends(int32 LocalUserNum, FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Friends) override;
	bool SendSessionInviteToFriends(const FUniqueNetId& LocalUserId, FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Friends) override;
	bool GetResolvedConnectString(FName SessionName, FString& ConnectInfo, FName PortType = NAME_GamePort) override;
	bool GetResolvedConnectString(const class FOnlineSessionSearchResult& SearchResult, FName PortType, FString& ConnectInfo) override;
	FOnlineSessionSettings* GetSessionSettings(FName SessionName) override;
	bool RegisterPlayer(FName SessionName, const FUniqueNetId& PlayerId, bool bWasInvited) override;
	bool RegisterPlayers(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Players, bool bWasInvited = false) override;
	bool UnregisterPlayer(FName SessionName, const FUniqueNetId& PlayerId) override;
	bool UnregisterPlayers(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Players) override;
	void RegisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnRegisterLocalPlayerCompleteDelegate& Delegate) override;
	void UnregisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnUnregisterLocalPlayerCompleteDelegate& Delegate) override;
	int32 GetNumSessions() override;
	void DumpSessionState() override;
	//~ End IOnlineSession overrides

	/**
	 * Query for any pending invites that the player passed in has yet to respond to.
	 */
	bool QueryAllInvites(const FUniqueNetId& PlayerId);

	/**
	 * Get every pending invite that has not yet been acted on
	 */
	TArray<FOnlineSessionInviteAccelByte> GetAllInvites() const;

	/**
	 * Get every pending game session invite that has not yet been acted on
	 */
	TArray<FOnlineSessionInviteAccelByte> GetAllGameInvites() const;

	/**
	 * Get every pending party invite that has not yet been acted on
	 */
	TArray<FOnlineSessionInviteAccelByte> GetAllPartyInvites() const;

	/**
	 * Rejects any invite that was sent to the player passed in.
	 */
	bool RejectInvite(const FUniqueNetId& PlayerId, const FOnlineSessionInviteAccelByte& InvitedSession);

	/**
	 * Query the backend for any session that the player is marked as active in. This is intended to be used to reconcile
	 * state between backend and client. Typically, this means that at login we want to grab all of the sessions where the
	 * player is marked as active to figure out if they are still considered to be in any sessions. From here, it is up
	 * to you if you want to rejoin the session through JoinSession, or leave the session entirely through the custom
	 * LeaveSession call.
	 * 
	 * @param LocalUserId ID of the user that we are restoring sessions for
	 * @param Delegate Handler fired after the restore call either succeeds or fails
	 */
	bool RestoreActiveSessions(const FUniqueNetId& LocalUserId, const FOnRestoreActiveSessionsComplete& Delegate = FOnRestoreActiveSessionsComplete());

	/**
	 * Grab every restored session from our cache
	 */
	TArray<FOnlineRestoredSessionAccelByte> GetAllRestoredSessions() const;

	/**
	 * Grab every restored party session from our cache
	 */
	TArray<FOnlineRestoredSessionAccelByte> GetAllRestoredPartySessions() const;

	/**
	 * Grab every restored game session from our cache
	 */
	TArray<FOnlineRestoredSessionAccelByte> GetAllRestoredGameSessions() const;

	/**
	 * Leave a restored session on the backend. For party sessions, this will allow you to create a new party instance
	 * once this call is finished. Though, this should also be done for game sessions that will not be rejoined.
	 */
	bool LeaveRestoredSession(const FUniqueNetId& LocalUserId, const FOnlineRestoredSessionAccelByte& SessionToLeave, const FOnLeaveSessionComplete& Delegate);

	/**
	 * Returns whether the local user is in a party session or not
	 */
	bool IsInPartySession() const;

	/**
	 * Get the party session instance that the player is in, or nullptr if they are not in one
	 */
	FNamedOnlineSession* GetPartySession() const;

	/*
	 * Attempt to get a named session instance by it's backend ID
	 */
	FNamedOnlineSession* GetNamedSessionById(const FString& SessionIdString);

	/**
	 * Register a dedicated server to Armada, either as a local server for testing, or as a managed Armada pod.
	 */
	void RegisterServer(FName SessionName, const FOnRegisterServerComplete& Delegate=FOnRegisterServerComplete());

	/**
	 * Unregister a dedicated server from Armada
	 */
	void UnregisterServer(FName SessionName, const FOnUnregisterServerComplete& Delegate = FOnUnregisterServerComplete());

	/**
	 * Get session type from session by its name
	 */
	EOnlineSessionTypeAccelByte GetSessionTypeByName(const FName& SessionName);

	/**
	 * Get an AccelByte session type from a session settings object
	 */
	EOnlineSessionTypeAccelByte GetSessionTypeFromSettings(const FOnlineSessionSettings& Settings) const;

	/**
	 * Try and get the ID of the user that is leader of this particular session. Only works with party sessions.
	 */
	FUniqueNetIdPtr GetSessionLeaderId(const FNamedOnlineSession* Session) const;

	/**
	 * Kick a member of the session out of the session.
	 */
	bool KickPlayer(const FUniqueNetId& LocalUserId, const FName& SessionName, const FUniqueNetId& PlayerIdToKick);

	/**
	 * Convert an array of region strings to a single list string for use in session settings.
	 */
	FString ConvertToRegionListString(const TArray<FString>& Regions) const;

	/**
	 * Convert a region list string back into an array of regions
	 */
	TArray<FString> ConvertToRegionArray(const FString& RegionListString) const;

	/**
	 * Get the list of regions that you are able to spin up a server in, sorted by latency to the player.
	 */
	TArray<FString> GetRegionList(const FUniqueNetId& LocalPlayerId) const;

	/**
	* Get the current session search handle that we are using for matchmaking.
	*/
	TSharedPtr<FOnlineSessionSearchAccelByte> GetCurrentMatchmakingSearchHandle() const;

	/**
	 * Delegate fired when we have retrieved information on the session that our server is associated with on the backend.
	 *
	 * @param SessionName the name that our server session is stored under
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnGetServerAssociatedSessionComplete, FName /*SessionName*/);

	/**
	 * Delegate fired when we have finished restoring all invites from the backend to the local interface.
	 *
	 * @param bWasSuccessful whether the call to get all invites was a success
	 * @param PlayerId ID of the player who had their invites restored
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnQueryAllInvitesComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*PlayerId*/);

	/**
	 * Delegate fired when we get a session invite from another player. Includes an AccelByte invite structure to allow for rejecting the invite.
	 * 
	 * @param UserId ID of the local user that received the invite
	 * @param FromId ID of the remote user that sent the invite
	 * @param Invite Invite structure that can be used to either accept or reject the invite
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnV2SessionInviteReceived, const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FromId*/, const FOnlineSessionInviteAccelByte& /*Invite*/);

	/**
	 * Delegate fired when our local list of invites has been updated
	 */
	DEFINE_ONLINE_DELEGATE(OnInviteListUpdated);

	/**
	 * Delegate fired when the server info for the session referenced changes. Basically is a signal that the server is
	 * ready to be traveled to.
	 * 
	 * @param SessionName Name of the session that has had server information change.
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnSessionServerUpdate, FName /*SessionName*/);

	/**
	 * Delegate fired when matchmaking has started
	 */
	DEFINE_ONLINE_DELEGATE(OnMatchmakingStarted);

PACKAGE_SCOPE:
	/** Restored sessions stored in this interface */
	TArray<FOnlineRestoredSessionAccelByte> RestoredSessions;

	/** Session invites stored in this interface */
	TArray<FOnlineSessionInviteAccelByte> SessionInvites;

	/**
	 * Current session search handle that we are using for matchmaking.
	 * 
	 * #NOTE (Maxwell): Since we only track one handle at a time, we don't support matchmaking for two different session names at the same time.
	 * This might be something useful down the line, think about if this is possible to support.
	 */
	TSharedPtr<FOnlineSessionSearchAccelByte> CurrentMatchmakingSearchHandle{nullptr};

	/**
	 * Session settings that we will create the matchmaking session result with.
	 */
	FOnlineSessionSettings CurrentMatchmakingSessionSettings{};

	FOnlineSessionV2AccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	/**
	 * Initializer method for this interface. Used to define internal delegate handlers that don't require user identifiers.
	 */
	void Init();

	/**
	 * Session tick for various background tasks
	 */
	void Tick(float DeltaTime);

	/**
	 * Register notification delegates for the player specified. Will also fire off associated OSS delegates once received.
	 */
	void RegisterSessionNotificationDelegates(const FUniqueNetId& PlayerId);

	/**
	 * Get an AccelByte joinability enum from a string value
	 */
	EAccelByteV2SessionJoinability GetJoinabilityFromString(const FString& JoinabilityStr) const;

	/**
	 * Get an AccelByte session type from a string value
	 */
	EOnlineSessionTypeAccelByte GetSessionTypeFromString(const FString& SessionTypeStr) const;

	/**
	 * Figure out the AccelByte joinability status of a session from its settings
	 */
	EAccelByteV2SessionJoinability GetJoinabiltyFromSessionSettings(const FOnlineSessionSettings& Settings) const;

	/**
	 * Figure out the AccelByte joinability status of a party session from its settings
	 */
	EAccelByteV2SessionJoinability GetPartyJoinabiltyFromSessionSettings(const FOnlineSessionSettings& Settings);
	
	/**
	 * Convenience method to return the string version of the joinability status
	*/
	FString V2SessionJoinabilityToString(const EAccelByteV2SessionJoinability& JoinType);

	/**
	 * Create a new named game session instance based on the passed in settings and backend session structure
	 */
	void FinalizeCreateGameSession(const FName& SessionName, const FAccelByteModelsV2GameSession& BackendSessionInfo);

	/**
	* Create a new named party session instance based on the passed in settings and backend session structure
	*/
	void FinalizeCreatePartySession(const FName& SessionName, const FAccelByteModelsV2PartySession& BackendSessionInfo);

	/**
	 * Construct a new session search result instance from a backend representation of a game session
	 */
	bool ConstructGameSessionFromBackendSessionModel(const FAccelByteModelsV2GameSession& BackendSession, FOnlineSession& OutResult);

	/**
	 * Construct a new session search result instance from a backend representation of a party session
	 */
	bool ConstructPartySessionFromBackendSessionModel(const FAccelByteModelsV2PartySession& BackendSession, FOnlineSession& OutResult);

	/**
	 * Checks whether or not the attribute name passed in is intended to be ignored for session attributes on backend.
	 */
	bool ShouldSkipAddingFieldToSessionAttributes(const FName& FieldName) const;

	/**
	 * Attempt to get the currently bound local address for a dedicated server.
	 */
	bool GetServerLocalIp(FString& OutIp) const;

	/**
	 * Attempt to get the currently bound port for a dedicated server.
	 */
	bool GetServerPort(int32& OutPort) const;

	/**
	 * Attempt to get the name of the local server to register with
	 */
	bool GetLocalServerName(FString& OutServerName) const;

	/**
	 * Attempt to get the session that is currently associated with this dedicated server. Will create a new game session
	 * under NAME_GameSession for the developer to interface with.
	 */
	bool GetServerAssociatedSession(FName SessionName);

	/**
	 * Remove a restored session instance by it's ID, if an instance exists
	 * 
	 * @param SessionIdStr Session ID that we are trying to find and remove a restore session instance for
	 * @return bool true if found and removed, false otherwise
	 */
	bool RemoveRestoreSessionById(const FString& SessionIdStr);

	/**
	 * Remove an invite from our local list by it's ID. Used when we accept or reject an invite.
	 * 
	 * @param SessionIdStr Session ID that we are trying to find and remove an invite instance for
	 * @return bool true if found and removed, false otherwise
	 */
	bool RemoveInviteById(const FString& SessionIdStr);

	/**
	 * Send a request on the backend to leave a session by its ID. Used internally by LeaveRestoredSession and DestroySession
	 * to notify backend that we want to leave the session referenced.
	 *
	 * @param LocalUserId ID of the user that is leaving the session provided
	 * @param SessionId ID of the session that we want to leave
	 * @param Delegate Delegate fired once the leave session call completes
	 * @return bool true if leave session task was spawned, false otherwise
	 */
	bool LeaveSession(const FUniqueNetId& LocalUserId, const EOnlineSessionTypeAccelByte& SessionType, const FString& SessionId, const FOnLeaveSessionComplete& Delegate=FOnLeaveSessionComplete());

	/**
	 * Convert a session settings object into a JSON object that can be used with create or update requests for sessions.
	 */
	TSharedRef<FJsonObject> ConvertSessionSettingsToJsonObject(const FOnlineSessionSettings& Settings) const;

	/**
	 * Read a JSON object into a session settings instance
	 */
	FOnlineSessionSettings ReadSessionSettingsFromJsonObject(const TSharedRef<FJsonObject>& Object) const;

	/**
	 *	Convert a session search parameters into a json object that can be used to fill match ticket attributes
	 */
	TSharedRef<FJsonObject> ConvertSearchParamsToJsonObject(const FSearchParams& Params) const;

private:
	/** Parent subsystem of this interface instance */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

	/** Critical section to lock sessions map while accessing */
	mutable FCriticalSection SessionLock;

	/** Sessions stored in this interface, associated by session name */
	TMap<FName, TSharedPtr<FNamedOnlineSession>> Sessions;

	/** Flag denoting whether there is already a task in progress to get a session associated with a server */
	bool bIsGettingServerAssociatedSession{ false };

	/** Array of delegates that are awaiting server session retrieval before executing */
	TArray<TFunction<void()>> SessionCallsAwaitingServerSession;

	/**
	 * Pointer to SessionSearch.
	 * will be set when matchmaking is in progress 
	 */
	TSharedPtr<FOnlineSessionSearch> SessionSearchPtr;

	/** Default session setting to use when no CustomSessionSetting is provided */
	FOnlineSessionSettings DefaultSessionSettings;

	/** 
	 * Session setting provided by user when matchmaking,
	 * if provided, CustomSessionSetting will be prioritized then DefaultSessionSetting.
	 * These will be cleared when matchmaking is complete and session search result is constructed
	 */
	TSharedPtr<FOnlineSessionSettings> ExplicitSessionSettings;

	/** Cache matchmaking ticket after started matchmaking. to be used to cancel matchmaking process */
	FString MatchmakingTicketId;

	/** Hidden on purpose */
	FOnlineSessionV2AccelByte() :
		AccelByteSubsystem(nullptr)
	{
	}

	bool CreatePartySession(const FUniqueNetId& HostingPlayerId, const FName& SessionName, const FOnlineSessionSettings& NewSessionSettings);
	bool CreateGameSession(const FUniqueNetId& HostingPlayerId, const FName& SessionName, const FOnlineSessionSettings& NewSessionSettings, bool bSendCreateRequest=true);

	//~ Begin Game Session Notification Handlers
	void OnInvitedToGameSessionNotification(FAccelByteModelsV2GameSessionUserInvitedEvent InviteEvent, int32 LocalUserNum);
	void OnFindGameSessionForInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result);
	void OnGameSessionMembersChangedNotification(FAccelByteModelsV2GameSessionMembersChangedEvent MembersChangedEvent, int32 LocalUserNum);
	void OnGameSessionUpdatedNotification(FAccelByteModelsV2GameSession UpdatedGameSession, int32 LocalUserNum);
	void OnDsStatusChangedNotification(FAccelByteModelsV2DSStatusChangedNotif DsStatusChangeEvent, int32 LocalUserNum);
	//~ End Game Session Notification Handlers

	//~ Begin Party Session Notification Handlers
	void OnInvitedToPartySessionNotification(FAccelByteModelsV2PartyInvitedEvent InviteEvent, int32 LocalUserNum);
	void OnFindPartySessionForInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result, FAccelByteModelsV2PartyInvitedEvent InviteEvent);
	void OnPartySessionMembersChangedNotification(FAccelByteModelsV2PartyMembersChangedEvent MemberChangeEvent, int32 LocalUserNum);
	void OnPartySessionUpdatedNotification(FAccelByteModelsV2PartySession UpdatedPartySession, int32 LocalUserNum);
	void OnPartySessionInviteRejectedNotification(FAccelByteModelsV2PartyUserRejectedEvent RejectEvent, int32 LocalUserNum);
	//~ End Party Session Notification Handlers

	//~ Begin Matchmaking Notification Handlers
	void OnMatchmakingStartedNotification(FAccelByteModelsV2StartMatchmakingNotif MatchmakingStartedNotif, int32 LocalUserNum);
	void OnMatchmakingMatchFoundNotification(FAccelByteModelsV2MatchFoundNotif MatchFoundEvent, int32 LocalUserNum);
	void OnFindMatchmakingGameSessionByIdComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& Result);
	//~ End Matchmaking Notification Handlers

	/**
	 * Generically handle members change events for party and game sessions.
	 */
	void HandleSessionMembersChangedNotification(const FString& SessionId, const TArray<FAccelByteModelsV2SessionUser>& NewMembers, const FString& JoinerId);

	void RegisterJoinedSessionMember(FNamedOnlineSession* Session, const FAccelByteModelsV2SessionUser& JoinedMember);
	void UnregisterLeftSessionMember(FNamedOnlineSession* Session, const FAccelByteModelsV2SessionUser& LeftMember);

	void OnGetServerAssociatedSessionComplete_Internal(FName SessionName);

protected:
	FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSessionSettings& SessionSettings) override;
	FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSession& Session) override;

	// Making this async task a friend so that it can add new named sessions
	friend class FOnlineAsyncTaskAccelByteGetServerAssociatedSessionV2;
};

typedef TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> FOnlineSessionV2AccelBytePtr;
