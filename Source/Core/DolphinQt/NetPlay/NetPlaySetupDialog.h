// Copyright 2017 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDialog>

#include "DolphinQt/GameList/GameListModel.h"

//Steam includes
#include <steam/steam_api.h>
#include <steam/isteammatchmaking.h>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QGridLayout;
class QPushButton;
class QSpinBox;
class QTabWidget;

namespace UICommon
{
class GameFile;
}

//defines a lobby
//struct KARLobby
//{
//  CSteamID lobbyID; //the steam ID for the lobby
//
//  std::string lobbyName = "", description = "", hostName = "", //Lobby data
//    ROMName = ""; //the name of the ROM, all Players must have the ROM with the same name
//};

class NetPlaySetupDialog : public QDialog
{
  //QT GUI macro bull shit
  Q_OBJECT

    // the callbacks for when a lobby is requested
    //CCallResult<NetPlaySetupDialog, LobbyMatchList_t> steamCallResult_LobbyMatchList;

  //  bool isRequestingLobbies = false,  // are the lobbies currently being requested
   //     lobbyListIsDirty = false,      // has the list changed and will need to be rebuilt
    //    isInLobbyMakerMenu = true;     // is it in the lobby maker menu or the lobby menu
   // std::vector<KARLobby> lobbies; //the KAR lobbies

    //--General Widgets For All
    QLineEdit* onlineNickNameInputFeild; //the input feild for the Player's custom nickname
    QGridLayout* windowLayout; //general layout of the whole window

    //---Build A Lobby Widgets
    QPushButton* startHostingGameButton;  // the button for starting to host a game
    QListWidget* modList;               // the list of mods

    QLineEdit* descriptionInputFeild; //the input feild for the lobby description
    QLabel* descriptionLabel;  // the input feild for the lobby description

    QLineEdit* lobbyNameInputFeild; //the input feild for the lobby name
    QLabel* lobbyNameLabel;   // the label for the lobby name

    QLineEdit* maxPlayerInputFeild; //the input feild for the lobby max Player
    QLabel* maxPLayerLabel; //the label for the max players

     QLineEdit* lobbyPasswordInputFeild;  // the input feild for the lobby password
      QCheckBox* lobbyPasswordCheckbox;          // the checkbox for the lobby password

    QLabel* portLabel; //the label for the port to use
    QLineEdit* portInputFeild; //the port to use

    QComboBox* gameCatagoryDropDown; //the drop down for if the lobby is for Ranking, Casual, or Other
    QComboBox* gameFlavorDropDown; //the drop down for if lobby is for Vannila KAR or Hack Pack KAR
    QComboBox* gameModeDropDown; //the drop down for if the lobby is for City Trial, Air Ride, or Top Ride

    //----functions

    //when the lobby request returns data
   // void OnLobbyMatchListCallback(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure);

    //creates the layout of the window
    void CreateWindowLayout();

public:

  //Constructor
  explicit NetPlaySetupDialog(const GameListModel& game_list_model, QWidget* parent);

  //refreshes the list of lobbies
 // inline void RefreshLobbies()
 // {
 //   if (!isRequestingLobbies)
 //   {
 //     isRequestingLobbies = true;
 //     // request all lobbies for this game
 //     SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterWorldwide);
 //    // SteamMatchmaking()->AddRequestLobbyListNearValueFilter(const char* pchKeyToMatch,
 //      //                                                           int nValueToBeCloseTo);
 //     SteamMatchmaking()->AddRequestLobbyListStringFilter("KAR_GameMode", "City Trial", ELobbyComparison::k_ELobbyComparisonEqual);
 //
 //     SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();
 //     // set the function to call when this API call has completed
 //     steamCallResult_LobbyMatchList.Set(hSteamAPICall, this,
 //                                         &NetPlaySetupDialog::OnLobbyMatchListCallback);
 //
 //     //update the lobby list to show it's updating
 //     lobbyList->clear();
 //     lobbyList->addItem(QString(tr("---GETTING LOBBIES---")));
 //   }
 // }

  //rebuilds the lobby list GUI
  //inline void RebuildLobbyList()
  //{
  //  if (!lobbyListIsDirty)
  //    return;
  //
  //  lobbyList->clear();
  //  for (uint32 i = 0; i < lobbies.size(); ++i)
  //    lobbyList->addItem(QString(tr(lobbies[i].lobbyName.c_str())));
  //
  //  lobbyListIsDirty = false;
  //}

  void accept() override;

  //renders the GUI
  void show();

signals:
 // bool Join();
  bool Host(const UICommon::GameFile& game);

private:
  //void CreateMainLayout();
  void ConnectWidgets();
  //void PopulateGameList();

  //void ResetTraversalHost();

  void SaveSettings();

 // void OnConnectionTypeChanged(int index);

  // Main Widget
 // QDialogButtonBox* m_button_box;
  //QComboBox* m_connection_type;
 // QLineEdit* m_nickname_edit;
  //QGridLayout* m_main_layout;
  //QTabWidget* m_tab_widget;

  //Lobby List Widgets
  //QPushButton* refreshLobbyListButton; //the button for refreshing the data
 // QListWidget* lobbyList; //the list of lobbies to render
  //QLabel* m_ip_label;
  //QLineEdit* m_ip_edit;
  //QLabel* m_connect_port_label;
  //QSpinBox* m_connect_port_box;
 // QPushButton* m_connect_button;

  //Lobby Hoster Widgets
  //QLabel* m_host_port_label;
  //QSpinBox* m_host_port_box;
 // QListWidget* m_host_games;
  //QPushButton* m_host_button;
 // QCheckBox* m_host_force_port_check;
 // QSpinBox* m_host_force_port_box;
 // QCheckBox* m_host_chunked_upload_limit_check;
 // QSpinBox* m_host_chunked_upload_limit_box;
 // QCheckBox* m_host_server_browser;
 // QLineEdit* m_host_server_name;
 // QLineEdit* m_host_server_password;
 // QComboBox* m_host_server_region;

  //CCallResult<NetPlaySetupDialog, LobbyMatchList_t> m_CallResultLobbyMatchList;  // the callbacks for when a lobby is requested


//#ifdef USE_UPNP
 // QCheckBox* m_host_upnp;
//#endif

  const GameListModel& m_game_list_model;
};
