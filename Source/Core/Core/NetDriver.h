#pragma once

// defines the Net Driver, the replacement for the Client and Server and general multiplayer

#include <Core/SyncIdentifier.h>

#include <string>

#include <steam/steam_api.h>
#include <steam/isteammatchmaking.h>

//defines the max name allowed online
#define NETPLAY_LIMIT_NAME_LENGTH 30U

namespace NetPlay::CustomBackend
{
// defines the KAR game mode
enum class KAR_GameMode
{
  City_Trial = 0,
  Air_Ride,
  Top_Ride,

  Count
};

// converts the game mode to a string
inline const char* KARGameModeToString(const KAR_GameMode mode)
{
  switch (mode)
  {
  case KAR_GameMode::City_Trial:
    return "City Trial";
  case KAR_GameMode::Air_Ride:
    return "Air Ride";
  case KAR_GameMode::Top_Ride:
    return "Top Ride";

  default:
    return "Count";
  }
}

// converts the game mode string to a game mode
inline KAR_GameMode StringToKARGameMode(const char* str)
{
  if (!strcmp(str, "City Trial"))
    return KAR_GameMode::City_Trial;
  else if (!strcmp(str, "Air Ride"))
    return KAR_GameMode::Air_Ride;
  if (!strcmp(str, "Top Ride"))
    return KAR_GameMode::Top_Ride;

  return KAR_GameMode::Count;
}

// defines the KAR game catagory
enum class KAR_GameCatagory
{
  Ranked = 0,
  Casual,
  Other,

  Count
};

// converts the game catagory to a string
inline const char* KARGameCatagoryToString(const KAR_GameCatagory catagory)
{
  switch (catagory)
  {
  case KAR_GameCatagory::Ranked:
    return "Ranked";
  case KAR_GameCatagory::Casual:
    return "Casual";
  case KAR_GameCatagory::Other:
    return "Other";

  default:
    return "Count";
  }
}

// converts a string to a game catagory
inline KAR_GameCatagory StringToKARGameCatagory(const char* str)
{
  if (!strcmp(str, "Ranked"))
    return KAR_GameCatagory::Ranked;
  else if (!strcmp(str, "Casual"))
    return KAR_GameCatagory::Casual;
  else if (!strcmp(str, "Other"))
    return KAR_GameCatagory::Other;

  return KAR_GameCatagory::Count;
}

// defines a Lobby
struct Lobby
{
  CSteamID lobbySteamID;  // the lobby Steam ID

  KAR_GameCatagory gameCatagory = KAR_GameCatagory::Ranked;  // the game catagory
  KAR_GameMode gameMode = KAR_GameMode::City_Trial;          // the game mode

  //uint8 maxPlayerCount = 2;  // the max amount of Players allowed
  //the current amount of Players in the lobby

  //the max number of spectators allowed
  //the current number of spectators

  //are spectators allowed
  //does the lobby have a password

  std::string name = ""; //the lobby name
  std::string desc = "";  // the lobby description
  //the lobby password

  std::string ROMName = "";  // the ROM being used

  //the mods

  //the gekko codes

  //updates the name in the steam lobby metadata
  inline void UpdateSteamLobbyMetadata_Name(const std::string& _name)
  {
    name = _name;
    SteamMatchmaking()->SetLobbyData(lobbySteamID, "name", name.c_str());
  }
  //updates the description in the steam lobby metadata
  inline void UpdateSteamLobbyMetadata_Description(const std::string& _desc)
  {
    desc = _desc;
    SteamMatchmaking()->SetLobbyData(lobbySteamID, "desc", desc.c_str());
  }

  // updates the host's region in the steam lobby metadata

  // updates the max player count in the steam lobby metadata
  // updates the current player count in the steam lobby metadata

  // updates the spectator max count in the steam lobby metadata
  // updates the current spectator count in the steam lobby metadata

  // updates the password in the steam lobby metadata
  // updates if the lobby has a password in the steam lobby metadata

  // updates the game catagory in the steam lobby metadata
  inline void UpdateSteamLobbyMetadata_GameCatagory(const KAR_GameCatagory catagory)
  {
    gameCatagory = catagory;
    SteamMatchmaking()->SetLobbyData(lobbySteamID, "game_catagory", KARGameCatagoryToString(gameCatagory));
  }
  // updates the game mode in the steam lobby metadata
  inline void UpdateSteamLobbyMetadata_GameMode(const KAR_GameMode mode)
  {
    gameMode = mode;
    SteamMatchmaking()->SetLobbyData(lobbySteamID, "game_mode",
                                     KARGameModeToString(gameMode));
  }
  // updates the game ROM in the steam lobby metadata
  inline void UpdateSteamLobbyMetadata_GameROM(const std::string _ROM)
  {
    ROMName = _ROM;
    SteamMatchmaking()->SetLobbyData(lobbySteamID, "game_ROM", ROMName.c_str());
  }

  // updates the mods in the steam lobby metadata

  // updates the gekko codes in the steam lobby metadata

  // gets the name from the steam lobby metadata
  inline void GetSteamLobbyMetadata_Name()
  {
    name = SteamMatchmaking()->GetLobbyData(lobbySteamID, "name");
  }
  // gets the description from the steam lobby metadata
  inline void GetSteamLobbyMetadata_Description()
  {
    desc = SteamMatchmaking()->GetLobbyData(lobbySteamID, "desc");
  }

  // gets the host's region from the steam lobby metadata

  // gets the max player count from the steam lobby metadata
  // gets the current player count from the steam lobby metadata

  // gets the spectator max count from the steam lobby metadata
  // gets the current spectator count from the steam lobby metadata

  // gets the password from the steam lobby metadata
  // gets if the lobby has a password from the steam lobby metadata

  // gets the game catagory from the steam lobby metadata
  inline void GetSteamLobbyMetadata_GameCatagory()
  {
    gameCatagory = StringToKARGameCatagory(SteamMatchmaking()->GetLobbyData(lobbySteamID, "game_catagory"));
  }
  // gets the game mode from the steam lobby metadata
  inline void GetSteamLobbyMetadata_GameMode()
  {
    gameMode = StringToKARGameMode(SteamMatchmaking()->GetLobbyData(lobbySteamID, "game_mode"));
  }
  // gets the game ROM from the steam lobby metadata
  inline void GetSteamLobbyMetadata_GameROM()
  {
    ROMName = SteamMatchmaking()->GetLobbyData(lobbySteamID, "game_ROM");
  }

  // gets the mods from the steam lobby metadata

  // gets the gekko codes from the steam lobby metadata
};

// defines a Player
struct Player
{
  CSteamID playerSteamID; //the Steam ID
  uint64 playerID; //the unique Player ID

  //is the Player a real Player or a spectator

  //the Player's Region
  //the Player's Ping
  
  std::string name = "";  // the player's nickname
};

// defines a client

// defines a host

// defines the netdriver
struct NetDriver
{
  bool isSteamInitalized = false;  // is steam initalized
  bool isHosting = false;  // is it the host
  bool isClient = false; //is it a client connecting to another

  Lobby lobby;  // the Lobby

  uint32 PlayerListIndex = 0;   // the index into the Player array that this driver belongs to

  //the total spectators
  std::vector<Player> players;  // the total Players

  //steam result callbacks
  CCallResult<NetDriver, LobbyCreated_t> steamCallResult_OnLobbyCreate;
  CCallResult<NetDriver, LobbyEnter_t> steamCallResult_OnLobbyEnter;

  //steam function callbacks
  void SteamCallbackFunc_LobbyCreated(LobbyCreated_t* callback, bool fail);
  void SteamCallbackFunc_LobbyEnter(LobbyEnter_t* callback, bool fail);

public:
  // Constructor
  NetDriver();

  // Deconstructor
  ~NetDriver();

  // starts hosting a lobby/game
  bool StartHosting();

  // sets the Game to be played
  bool ChangeGame(const NetPlay::SyncIdentifier& ROMsyncIdentifier,
                  const std::string& netplayROMName);

  //gets all the data of the lobby
  Lobby GetAllLobbyData(CSteamID lobbyID);
};

}  // namespace NetPlay::CustomBackend
