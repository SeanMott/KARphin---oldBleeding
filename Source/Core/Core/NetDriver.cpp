//impl file for Net Driver, the replacement for the Client and Server and general multiplayer

#include <Core/NetDriver.h>

#include "Core/Config/MainSettings.h"
#include "Core/Config/NetplaySettings.h"

//----STEAM FUNCTION CALLBACKS---//

void NetPlay::CustomBackend::NetDriver::SteamCallbackFunc_LobbyCreated(LobbyCreated_t* callback, bool fail)
{
  lobby.lobbySteamID = callback->m_ulSteamIDLobby;

  // updates the Lobby name
  lobby.UpdateSteamLobbyMetadata_Name(Config::CONFIG_SETTING_LOBBY_NAME);
  lobby.UpdateSteamLobbyMetadata_Description(Config::CONFIG_SETTING_LOBBY_DESC);

  // updates the game catagory

  // updates the game mode

  // updates the game ROM

  // updates the game mods
}

void NetPlay::CustomBackend::NetDriver::SteamCallbackFunc_LobbyEnter(LobbyEnter_t* callback, bool fail)
{
  //if it is the host, no point in grabbing the data
  if (isHosting)
    return;

  lobby = GetAllLobbyData(callback->m_ulSteamIDLobby); //gets all the lobby data
}

//------FUNCTIONS----///

// Constructor
NetPlay::CustomBackend::NetDriver::NetDriver()
{
  // init Steam
  bool SteamInit = SteamAPI_Init();
  SteamErrMsg errMsg;
  SteamAPI_InitEx(&errMsg);

  if (!SteamInit)
    return;

  // Initialize the peer to peer connection process.  This is not required, but we do it
  // because we cannot accept connections until this initialization completes, and so we
  // want to start it as soon as possible.
  SteamNetworkingUtils()->InitRelayNetworkAccess();

  isSteamInitalized = true;

  players.reserve(4);
}

// Deconstructor
NetPlay::CustomBackend::NetDriver::~NetDriver()
{
  // shutdown Steam
  SteamAPI_Shutdown();
}

// sets the Game to be played
bool NetPlay::CustomBackend::NetDriver::ChangeGame(const NetPlay::SyncIdentifier& ROMsyncIdentifier, const std::string& netplayROMName)
{
 // std::lock_guard lkg(m_crit.game);

 // INFO_LOG_FMT(NETPLAY, "Changing game to {} ({:02x}).", netplay_name,
  //             fmt::join(sync_identifier.sync_hash, ""));

  //m_selected_game_identifier = sync_identifier;
  //m_selected_game_name = netplay_name;

  // send changed game to clients
 // sf::Packet spac;
  //spac << MessageID::ChangeGame;
 // SendSyncIdentifier(spac, m_selected_game_identifier);
 // spac << m_selected_game_name;

 // SendAsyncToClients(std::move(spac));

  return true;
};

//starts hosting a lobby/game
bool NetPlay::CustomBackend::NetDriver::StartHosting()
{
  //creates the lobby and enters it
  SteamAPICall_t callback = SteamMatchmaking()->CreateLobby(ELobbyType::k_ELobbyTypePublic,
                                  Config::CONFIG_SETTING_LOBBY_MAX_PLAYER_COUNT);
  steamCallResult_OnLobbyCreate.Set(callback, this, &NetDriver::SteamCallbackFunc_LobbyCreated);
  steamCallResult_OnLobbyEnter.Set(callback, this, &NetDriver::SteamCallbackFunc_LobbyEnter);

  isHosting = true;
  return true;
}

// gets all the data of the lobby
NetPlay::CustomBackend::Lobby NetPlay::CustomBackend::NetDriver::GetAllLobbyData(CSteamID lobbyID)
{
  // allocate lobby
  Lobby _lobby;
  _lobby.lobbySteamID = lobbyID;

  _lobby.GetSteamLobbyMetadata_Name();
  _lobby.GetSteamLobbyMetadata_Description();

  //does the lobby have a password
  //gets the password

  //gets the max player count
  //get the current player count

  //are spectators allowed
  //is there a spectator limit
  //get the max spector count
  //gets the current spectator count

  _lobby.GetSteamLobbyMetadata_GameCatagory();
  _lobby.GetSteamLobbyMetadata_GameMode();
  _lobby.GetSteamLobbyMetadata_GameROM();

  // gets the game mods

  //gets the game gekko codes

  return _lobby;
}
