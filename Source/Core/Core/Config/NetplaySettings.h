// Copyright 2017 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <string>

#include "Common/CommonTypes.h"
#include "Common/Config/Config.h"

#include <Core/NetDriver.h>
//#include "Core/NetDriver.hpp"

namespace Config
{
// Configuration Information

// Main.NetPlay

  // defines configs about net play
extern std::string CONFIG_SETTING_USER_NICKNAME;

extern std::string CONFIG_SETTING_LOBBY_NAME;
extern std::string CONFIG_SETTING_LOBBY_DESC;

extern u8 CONFIG_SETTING_LOBBY_MAX_PLAYER_COUNT;

extern std::string CONFIG_SETTING_LOBBY_REGION;
extern std::string CONFIG_SETTING_LOBBY_PASSWORD;
extern bool CONFIG_SETTING_LOBBY_PASSWORD_IS_ENABLED;

extern u16 CONFIG_SETTING_LOBBY_HOST_PORT;
extern u16 CONFIG_SETTING_LOBBY_CLIENT_PORT;

extern NetPlay::CustomBackend::KAR_GameMode CONFIG_SETTING_GAME_MODE;
extern NetPlay::CustomBackend::KAR_GameCatagory CONFIG_SETTING_GAME_CATAGORY;
extern std::string CONFIG_SETTING_GAME_ROM_NAME;

extern CSteamID CONFIG_SETTING_LOBBY_STEAM_ID;

// mods

extern const Info<std::string> NETPLAY_TRAVERSAL_SERVER;
extern const Info<u16> NETPLAY_TRAVERSAL_PORT;
extern const Info<u16> NETPLAY_TRAVERSAL_PORT_ALT;
extern const Info<std::string> NETPLAY_TRAVERSAL_CHOICE;
extern const Info<std::string> NETPLAY_HOST_CODE;
extern const Info<std::string> NETPLAY_INDEX_URL;

extern const Info<u16> NETPLAY_HOST_PORT;
extern const Info<std::string> NETPLAY_ADDRESS;
extern const Info<u16> NETPLAY_CONNECT_PORT;
extern const Info<u16> NETPLAY_LISTEN_PORT;

extern const Info<std::string> NETPLAY_NICKNAME;
extern const Info<bool> NETPLAY_USE_UPNP;

extern const Info<bool> NETPLAY_ENABLE_QOS;

extern const Info<bool> NETPLAY_USE_INDEX;
extern const Info<std::string> NETPLAY_INDEX_REGION;
extern const Info<std::string> NETPLAY_INDEX_NAME;
extern const Info<std::string> NETPLAY_INDEX_PASSWORD;

extern const Info<bool> NETPLAY_ENABLE_CHUNKED_UPLOAD_LIMIT;
extern const Info<u32> NETPLAY_CHUNKED_UPLOAD_LIMIT;

extern const Info<u32> NETPLAY_BUFFER_SIZE;
extern const Info<u32> NETPLAY_CLIENT_BUFFER_SIZE;

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
