// Copyright 2017 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <string>

#include "Common/CommonTypes.h"
#include "Common/Config/Config.h"

#include <steam/isteammatchmaking.h>

namespace Config
{
// Configuration Information

// Main.NetPlay

  extern CSteamID NETPLAY_LOBBY_STEAM_ID;
extern std::string NETPLAY_LOBBY_NAME;
  extern std::string NETPLAY_LOBBY_REGION;

  extern bool NETPLAY_LOBBY_HAS_PASSWORD;
  extern std::string NETPLAY_LOBBY_PASSWORD;

  extern bool NETPLAY_LOBBY_IS_DIRECT_OR_TRAVERSAL;
  extern bool NETPLAY_LOBBY_USE_UPNP;
  extern bool NETPLAY_LOBBY_ENABLE_QOS;

  extern u16 NETPLAY_LOBBY_HOST_PORT;
  extern u16 NETPLAY_LOBBY_CONNECT_PORT;
  extern std::string NETPLAY_LOBBY_HOST_CODE_OR_IP;

  extern bool NETPLAY_ENABLE_CHUNKED_UPLOAD_LIMIT;
  extern u32 NETPLAY_CHUNKED_UPLOAD_LIMIT;

  extern u32 NETPLAY_BUFFER_SIZE;
  extern u32 NETPLAY_CLIENT_BUFFER_SIZE;

  extern std::string NETPLAY_USER_NICKNAME;

//extern const Info<std::string> NETPLAY_TRAVERSAL_SERVER;
//extern const Info<u16> NETPLAY_TRAVERSAL_PORT;
//extern const Info<u16> NETPLAY_TRAVERSAL_PORT_ALT;
//extern const Info<std::string> NETPLAY_TRAVERSAL_CHOICE;
//extern const Info<std::string> NETPLAY_HOST_CODE;
//extern const Info<std::string> NETPLAY_INDEX_URL;

//extern const Info<u16> NETPLAY_HOST_PORT;
//extern const Info<std::string> NETPLAY_ADDRESS;
//extern const Info<u16> NETPLAY_CONNECT_PORT;
//extern const Info<u16> NETPLAY_LISTEN_PORT;

//extern const Info<std::string> NETPLAY_NICKNAME;
//extern const Info<bool> NETPLAY_USE_UPNP;

//extern const Info<bool> NETPLAY_ENABLE_QOS;

//extern const Info<bool> NETPLAY_USE_INDEX;
//extern const Info<std::string> NETPLAY_INDEX_REGION;
//extern const Info<std::string> NETPLAY_INDEX_NAME;
//extern const Info<std::string> NETPLAY_INDEX_PASSWORD;

//extern const Info<bool> NETPLAY_ENABLE_CHUNKED_UPLOAD_LIMIT;
//extern const Info<u32> NETPLAY_CHUNKED_UPLOAD_LIMIT;

//extern const Info<u32> NETPLAY_BUFFER_SIZE;
//extern const Info<u32> NETPLAY_CLIENT_BUFFER_SIZE;

extern const Info<bool> NETPLAY_SAVEDATA_LOAD;
extern const Info<bool> NETPLAY_SAVEDATA_WRITE;
extern const Info<bool> NETPLAY_SAVEDATA_SYNC_ALL_WII;
extern const Info<bool> NETPLAY_SYNC_CODES;
extern const Info<bool> NETPLAY_RECORD_INPUTS;
extern const Info<bool> NETPLAY_STRICT_SETTINGS_SYNC;
extern const Info<std::string> NETPLAY_NETWORK_MODE;
extern const Info<bool> NETPLAY_GOLF_MODE_OVERLAY;
extern const Info<bool> NETPLAY_HIDE_REMOTE_GBAS;

}  // namespace Config
