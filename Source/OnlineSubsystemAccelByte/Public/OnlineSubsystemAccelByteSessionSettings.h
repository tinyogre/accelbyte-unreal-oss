// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

// These are all of the session settings used by the AccelByte Online Subsystem for our V2 sessions.

// GENERIC SESSIONS BEGIN
#define SETTING_SESSION_TYPE FName(TEXT("SESSIONTYPE"))
#define SETTING_SESSION_TYPE_PARTY_SESSION TEXT("PartySession")
#define SETTING_SESSION_TYPE_GAME_SESSION TEXT("GameSession")
#define SETTING_SESSION_JOIN_TYPE FName(TEXT("SESSIONJOINTYPE"))
#define SETTING_SESSION_MATCHPOOL FName(TEXT("MATCHPOOL"))
#define SETTING_SESSION_MINIMUM_PLAYERS FName(TEXT("MINPLAYERS"))
#define SETTING_SESSION_INACTIVE_TIMEOUT FName(TEXT("INACTIVETIMEOUT"))
#define SETTING_SESSION_INVITE_TIMEOUT FName(TEXT("INVITETIMEOUT"))
#define SETTING_SESSION_SERVER_TYPE FName(TEXT("SERVERTYPE"))
#define SETTING_SESSION_TEXTCHAT FName(TEXT("TEXTCHAT"))
#define SETTING_SESSION_CODE FName(TEXT("SESSIONCODE"))
#define SETTING_SESSION_TEAMS FName(TEXT("SESSIONTEAMS"))
// GENERIC SESSIONS END

// GAME SESSIONS BEGIN
#define SETTING_GAMESESSION_CLIENTVERSION FName(TEXT("CLIENTVERSION"))
#define SETTING_GAMESESSION_DEPLOYMENT FName(TEXT("DEPLOYMENT"))
#define SETTING_GAMESESSION_SERVERNAME FName(TEXT("SERVERNAME"))
#define SETTING_GAMESESSION_REQUESTEDREGIONS FName(TEXT("REQUESTEDREGIONS"))
// GAME SESSIONS END

// PARTY SESSIONS BEGIN
// keeping for backward compatibility, should use SETTING_SESSION_CODE going forward.
#define SETTING_PARTYSESSION_CODE FName(TEXT("PARTYCODE"))
// PARTY SESSIONS END

// MATCHMAKING BEGIN
#define SETTING_MATCHMAKING_SESSION_ID FName(TEXT("SESSIONID"))
#define SETTING_MATCHMAKING_BACKFILL_TICKET_ID FName(TEXT("BACKFILLTICKETID"))
// MATCHMAKING END
