#pragma once

//defines a custom Lobby type for KARphin net play for KAR Lobbies

#include <string>

#include <steam/isteammatchmaking.h>

namespace NetPlay::CustomBackend::KAR
{
	//defines a game mode
	enum class GameMode
	{
		City_Trial = 0,

		Air_Ride,

		Top_Ride,

		Count
	};

	//converts a game mode to a string
	inline const char* GameModeToStr(const GameMode mode)
	{
		switch (mode)
		{
    case GameMode::City_Trial:
      return "City Trial";

			case GameMode::Air_Ride:
      return "Air Ride";

			case GameMode::Top_Ride:
        return "Top Ride";
		}

		return "GAME_MODE_KAR_UNKNOWN";
	}

	//converts a string to a game mode
  inline GameMode StrToGameMode(const char* mode)
  {
    if (!strcmp(mode, "City Trial"))
      return GameMode::City_Trial;
    else if (!strcmp(mode, "Air Ride"))
      return GameMode::Air_Ride;
    else if (!strcmp(mode, "Top Ride"))
      return GameMode::Top_Ride;

    return GameMode::Count;
  }

	//defines a game catagory
	enum class GameCatagory
  {
    Ranked = 0,

    Casual,

    Other,

    Count
  };

	//converts a game catagory to string
  inline const char* GameCatagoryToStr(const GameCatagory cat)
  {
    switch (cat)
    {
    case GameCatagory::Ranked:
      return "Rank";

    case GameCatagory::Casual:
      return "Casual";

    case GameCatagory::Other:
      return "Other";
    }

    return "GAME_CATAGORY_KAR_UNKNOWN";
  }

	//converts a string to a game catagory
  inline GameCatagory StrToGameCatagory(const char* mode)
  {
    if (!strcmp(mode, "Rank"))
      return GameCatagory::Ranked;
    else if (!strcmp(mode, "Casual"))
      return GameCatagory::Casual;
    else if (!strcmp(mode, "Other"))
      return GameCatagory::Other;

    return GameCatagory::Other;
  }

  enum class GameStatus
  {
    Waiting = 0,
    TweakingMods,
    Ready,
    InGame,

    Count
  };

  //converts a game status to a string
  inline const char* GameStatusToStr(GameStatus status)
  {
    switch (status)
    {
    case GameStatus::Waiting:
      return "Waiting";

      case GameStatus::TweakingMods:
      return "Tweaking Status";

      case GameStatus::Ready:
        return "Ready";

        case GameStatus::InGame:
        return "In Game";
    }

    return "UNKNOWN_GAME_STATUS";
  }

  //converts a string to a game status
  inline GameStatus StrToGameStatus(const char* str)
  {
    if (!strcmp(str, "Waiting"))
      return GameStatus::Waiting;

    else if (!strcmp(str, "Tweaking Status")) return GameStatus::TweakingMods;

    else if (!strcmp(str, "Ready")) return GameStatus::Ready;

    else if (!strcmp(str, "In Game")) return GameStatus::InGame;

    return GameStatus::Count;
  }

  //defines the var name for the lobby name
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_NAME "name"
  // defines the var name for the lobby region
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_REGION "region"

  // defines the var name for the lobby game catagory
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_GAME_CATAGORY "game_catagory"
  // defines the var name for the lobby game mode
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_GAME_MODE "game_mode"
  // defines the var name for the lobby game status
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_GAME_STATUS "game_status"

  // defines the var name for the lobby max player count
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_MAX_PLAYER_COUNT "max_player_count"
  // defines the var name for the lobby current player count
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_CURRENT_PLAYER_COUNT "current_player_count"

  // defines the var name for the lobby KARphin ver
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_KARPHIN_VER "KARphin_ver"
  // defines the var name for the lobby game ROM ID
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_ROM_ID "ROM_ID"

  // defines the var name for the lobby password
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_PASSWORD "password"
  // defines the var name for the lobby if it has a password
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_HAS_PASSWORD "has_password"

  // defines the var name for the lobby if it uses traversal or direct
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_NETWORK_MODE_DIRECT_OR_TRAVERSAL "network_mode"
  // defines the var name for the lobby exposed host port
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_EXPOSED_HOST_PORT "host_port"
  // defines the var name for the lobby exposed host code or host IP
#define NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_EXPOSED_HOST_CODE_OR_IP "host_code"

  //defines a KAR Lobby
  struct Lobby
  {
    CSteamID lobbyID;

    GameCatagory gameCatagory = GameCatagory::Count;
    GameMode gameMode = GameMode::Count;
    GameStatus status = GameStatus::Count;

    bool isPassword = false;
    bool isNetModeDirect = false;  // is the Lobby using direct or Traversal

    uint16 port = 0;  // the port to connect to

    std::string hostAddress_IP = "";  // the IP/Host code to connect to

    std::string name = "", region = "", gameID = "", KARphinVer = "", password = "";

    // gets the name of a KAR lobby
    inline void Get_Name()
    {
      name = SteamMatchmaking()->GetLobbyData(lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_NAME);
    }
    // gets the region of a KAR lobby
    inline void Get_Region()
    {
      region = SteamMatchmaking()->GetLobbyData(lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_REGION);
    }
    // gets the ROM/Game ID of a KAR lobby
    inline void Get_ROMID()
    {
      gameID = SteamMatchmaking()->GetLobbyData(lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_ROM_ID);
    }

    // gets the KARphin version
    inline void Get_KARphinVer()
    {
      KARphinVer = SteamMatchmaking()->GetLobbyData(lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_KARPHIN_VER);
    }

    // gets the game mode of a KAR lobby
    inline void Get_GameMode()
    {
      gameMode = StrToGameMode(SteamMatchmaking()->GetLobbyData(lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_GAME_MODE));
    }
    // gets the game catagory of a KAR lobby
    inline void Get_GameCatagory()
    {
      gameCatagory = StrToGameCatagory(SteamMatchmaking()->GetLobbyData(lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_GAME_CATAGORY));
    }

    // gets if a KAR lobby has a password
    inline void Get_HasPassword()
    {
      isPassword = !(strcmp(SteamMatchmaking()->GetLobbyData(lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_HAS_PASSWORD), "true"));
    }
    // gets the KAR lobby password
    inline void Get_Password()
    {
      password = SteamMatchmaking()->GetLobbyData(lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_PASSWORD);
    }

    // gets the status of the game
    inline void Get_GameStatus()
    {
      status = StrToGameStatus(SteamMatchmaking()->GetLobbyData(lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_GAME_STATUS));
    }

    // gets if the game uses traversal or direct of the game
    inline void Get_NetworkMode()
    {
      isNetModeDirect = !(strcmp("direct", SteamMatchmaking()->GetLobbyData(lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_NETWORK_MODE_DIRECT_OR_TRAVERSAL)));
    }

    // gets the exposed host port
    inline void Get_HostPort()
    {
      port = atoi(SteamMatchmaking()->GetLobbyData(
          lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_EXPOSED_HOST_PORT));
    }

    // gets the exposed IP or Host code
    inline void Get_HostCode_IP()
    {
      hostAddress_IP = SteamMatchmaking()->GetLobbyData(
          lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_EXPOSED_HOST_CODE_OR_IP);
    }

    // sets the name of a KAR lobby
    inline void Set_Name(const std::string _name)
    {
      name = _name;
      SteamMatchmaking()->SetLobbyData(lobbyID,
                                              NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_NAME,
        name.c_str());
    }
    // sets the region of a KAR lobby
    inline void Set_Region(const std::string _region)
    {
      region = _region;
      SteamMatchmaking()->SetLobbyData(
          lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_REGION,
        region.c_str());
    }
    // sets the ROM/Game ID of a KAR lobby
    inline void Set_ROMID(const std::string _ROMID)
    {
      gameID = _ROMID;
      SteamMatchmaking()->SetLobbyData(
          lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_ROM_ID,
        gameID.c_str());
    }

    // sets the KARphin version
    inline void Set_KARphinVer(const std::string _KAR_ver)
    {
      KARphinVer = _KAR_ver;
      SteamMatchmaking()->SetLobbyData(
          lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_KARPHIN_VER,
        KARphinVer.c_str());
    }

    // sets the game mode of a KAR lobby
    inline void Set_GameMode(GameMode mode)
    {
      gameMode = mode;
      SteamMatchmaking()->SetLobbyData(
          lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_GAME_MODE,
        GameModeToStr(gameMode));
    }
    // sets the game catagory of a KAR lobby
    inline void Set_GameCatagory(GameCatagory cat)
    {
      gameCatagory = cat; 
      SteamMatchmaking()->SetLobbyData(
          lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_GAME_CATAGORY,
        GameCatagoryToStr(gameCatagory));
    }

    // sets if a KAR lobby has a password
    inline void Set_HasPassword(bool _isPassword)
    {
      isPassword = _isPassword;
      SteamMatchmaking()->SetLobbyData(lobbyID,
                                       NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_HAS_PASSWORD,
                                       (isPassword == true ? "true" : "false"));
    }
    // sets the KAR lobby password
    inline void Set_Password(const std::string _password)
    {
      password = _password;
        SteamMatchmaking()->SetLobbyData(
          lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_PASSWORD,
          password.c_str());
    }

    // sets the status of the game
    inline void Set_GameStatus(GameStatus _status)
    {
      status = _status;
        SteamMatchmaking()->SetLobbyData(
          lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_GAME_STATUS,
          GameStatusToStr(status));
    }

    // sets if the game uses traversal or direct of the game
    inline void Set_NetworkMode(const bool _netMode)
    {
      isNetModeDirect = _netMode;
          SteamMatchmaking()->SetLobbyData(
              lobbyID,
              NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_NETWORK_MODE_DIRECT_OR_TRAVERSAL,
          (isNetModeDirect == true ? "direct" : "traversal"));
    }

    // sets the exposed host port
    inline void Set_HostPort(const uint16 _port)
    {
      port = _port;
      SteamMatchmaking()->SetLobbyData(
          lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_EXPOSED_HOST_PORT,
        std::to_string(port).c_str());
    }

    // sets the exposed IP or Host code
    inline void Set_HostCode_IP(const std::string address)
    {
      hostAddress_IP = address;
      SteamMatchmaking()->SetLobbyData(
          lobbyID, NETPLAY_CUSTOM_BACKEND_KAR_LOBBY_METADATA_VAR_EXPOSED_HOST_CODE_OR_IP,
        hostAddress_IP.c_str());
    }
  };
  }