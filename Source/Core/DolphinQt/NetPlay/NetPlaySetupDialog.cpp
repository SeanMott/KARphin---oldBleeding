// Copyright 2017 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DolphinQt/NetPlay/NetPlaySetupDialog.h"

#include <memory>

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>

#include "Core/Config/NetplaySettings.h"
#include "Core/NetPlayProto.h"

#include "DolphinQt/QtUtils/ModalMessageBox.h"
#include "DolphinQt/QtUtils/NonDefaultQPushButton.h"
#include "DolphinQt/QtUtils/UTF8CodePointCountValidator.h"
#include "DolphinQt/Settings.h"

#include "UICommon/GameFile.h"
#include "UICommon/NetPlayIndex.h"

#include <Core/NetDriver.h>

//Steam includes
#include <steam/steam_api.h>
#include <steam/isteammatchmaking.h>

NetPlaySetupDialog::NetPlaySetupDialog(const GameListModel& game_list_model, QWidget* parent)
    : QDialog(parent), m_game_list_model(game_list_model)
{
  setWindowTitle(
      tr("Karphin Online"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  //checks if steam is initalized
  if (!Settings::Instance().GetNetDriver()->isSteamInitalized)
  {
    ModalMessageBox::information(this, tr("Net Play Error"),
                                 tr("Steam Networking failed to initalized. Make sure Steam is "
                                    "running before starting netplay."));
    return;
  }

  //create the main window layout
  CreateWindowLayout();

  //CreateMainLayout();

  //bool use_index = Config::Get(Config::NETPLAY_USE_INDEX);
  //std::string index_region = Config::Get(Config::NETPLAY_INDEX_REGION);
  //std::string index_name = Config::Get(Config::NETPLAY_INDEX_NAME);
  //std::string index_password = Config::Get(Config::NETPLAY_INDEX_PASSWORD);
  //std::string nickname = Config::Get(Config::NETPLAY_NICKNAME);
  //std::string traversal_choice = Config::Get(Config::NETPLAY_TRAVERSAL_CHOICE);
  //int connect_port = Config::Get(Config::NETPLAY_CONNECT_PORT);
  //int host_port = Config::Get(Config::NETPLAY_HOST_PORT);
  //int host_listen_port = Config::Get(Config::NETPLAY_LISTEN_PORT);
  //bool enable_chunked_upload_limit = Config::Get(Config::NETPLAY_ENABLE_CHUNKED_UPLOAD_LIMIT);
  //u32 chunked_upload_limit = Config::Get(Config::NETPLAY_CHUNKED_UPLOAD_LIMIT);
//#ifdef USE_UPNP
  //bool use_upnp = Config::Get(Config::NETPLAY_USE_UPNP);

  //m_host_upnp->setChecked(use_upnp);
//#endif

  //m_nickname_edit->setText(QString::fromStdString(nickname));
 // m_connection_type->setCurrentIndex(traversal_choice == "direct" ? 0 : 1);
  //m_connect_port_box->setValue(connect_port);
  //m_host_port_box->setValue(host_port);

  //m_host_force_port_box->setValue(host_listen_port);
  //m_host_force_port_box->setEnabled(false);

  //m_host_server_browser->setChecked(use_index);

  //m_host_server_region->setEnabled(use_index);
  //m_host_server_region->setCurrentIndex(
  //    m_host_server_region->findData(QString::fromStdString(index_region)));

  //m_host_server_name->setEnabled(use_index);
  //m_host_server_name->setText(QString::fromStdString(index_name));

  //m_host_server_password->setEnabled(use_index);
  //m_host_server_password->setText(QString::fromStdString(index_password));

  //m_host_chunked_upload_limit_check->setChecked(enable_chunked_upload_limit);
  //m_host_chunked_upload_limit_box->setValue(chunked_upload_limit);
  //m_host_chunked_upload_limit_box->setEnabled(enable_chunked_upload_limit);

  //OnConnectionTypeChanged(m_connection_type->currentIndex());

  ConnectWidgets();
}

// when the lobby request returns data
//void NetPlaySetupDialog::OnLobbyMatchListCallback(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure)
//{
//  lobbies.clear();
//  isRequestingLobbies = false;
//
//  if (bIOFailure)
//  {
//    // we had a Steam I/O failure - we probably timed out talking to the Steam back-end servers
//    // doesn't matter in this case, we can just act if no lobbies were received
//  }
//
//  // lobbies are returned in order of closeness to the user, so add them to the list in that order
//  for (uint32 iLobby = 0; iLobby < pLobbyMatchList->m_nLobbiesMatching; iLobby++)
//  {
//    CSteamID steamIDLobby = SteamMatchmaking()->GetLobbyByIndex(iLobby);
//
//    // add the lobby to the list
//    KARLobby lobby;
//    lobby.lobbyID = steamIDLobby;
//    // pull the name from the lobby metadata
//    const char* pchLobbyName = SteamMatchmaking()->GetLobbyData(steamIDLobby, "name");
//    if (pchLobbyName && pchLobbyName[0])
//    {
//      // set the lobby name
//      lobby.lobbyName = std::string(pchLobbyName);
//    }
//    else
//    {
//      // we don't have info about the lobby yet, request it
//      SteamMatchmaking()->RequestLobbyData(steamIDLobby);
//      // results will be returned via LobbyDataUpdate_t callback
//      //sprintf_safe(lobby.m_rgchName, "Lobby %d", steamIDLobby.GetAccountID());
//      //lobby.lobbyName = steamIDLobby.GetAccountID();
//
//      //skip it after requesting more data
//      continue;
//    }
//
//    lobbies.emplace_back(lobby);
//  }
//
//  lobbyListIsDirty = true;
//}

void NetPlaySetupDialog::CreateWindowLayout()
{
  //initalize general widgets
  windowLayout = new QGridLayout;
  //windowTabLayout = new QTabWidget;

  onlineNickNameInputFeild = new QLineEdit;
  onlineNickNameInputFeild->setValidator(
      new UTF8CodePointCountValidator(NetPlay::MAX_NAME_LENGTH, onlineNickNameInputFeild));

  // adds nickname feild
  windowLayout->addWidget(onlineNickNameInputFeild, 0, 1);
  windowLayout->addWidget(new QLabel(tr("Nickname:")), 0, 0);

  //adds lobby name
  lobbyNameLabel = new QLabel();
  lobbyNameLabel->setText(tr("Name"));
  windowLayout->addWidget(lobbyNameLabel, 1, 0, 1, -1);

  lobbyNameInputFeild = new QLineEdit();
  lobbyNameInputFeild->setText(tr("Lobby 1"));
  windowLayout->addWidget(lobbyNameInputFeild, 1, 1, 1, -1);

  //adds lobby description
  descriptionLabel = new QLabel();
  descriptionLabel->setText(tr("Desc"));
  windowLayout->addWidget(descriptionLabel, 2, 0, 1, -1);

  descriptionInputFeild = new QLineEdit();
  descriptionInputFeild->setText(tr("KAR Lobby"));
  windowLayout->addWidget(descriptionInputFeild, 2, 1, 1, -1);

  //adds catagory
  gameCatagoryDropDown = new QComboBox();
  gameCatagoryDropDown->addItem(tr(NetPlay::CustomBackend::KARGameCatagoryToString(NetPlay::CustomBackend::KAR_GameCatagory::Ranked)));
  gameCatagoryDropDown->addItem(tr(NetPlay::CustomBackend::KARGameCatagoryToString(NetPlay::CustomBackend::KAR_GameCatagory::Casual)));
  gameCatagoryDropDown->addItem(tr(NetPlay::CustomBackend::KARGameCatagoryToString(NetPlay::CustomBackend::KAR_GameCatagory::Other)));
  windowLayout->addWidget(gameCatagoryDropDown, 3, 0, 1, -1);

  //adds the game mode
  gameModeDropDown = new QComboBox();
  gameModeDropDown->addItem(tr(NetPlay::CustomBackend::KARGameModeToString(NetPlay::CustomBackend::KAR_GameMode::City_Trial)));
  gameModeDropDown->addItem(tr(NetPlay::CustomBackend::KARGameModeToString(NetPlay::CustomBackend::KAR_GameMode::Air_Ride)));
  gameModeDropDown->addItem(tr(NetPlay::CustomBackend::KARGameModeToString(NetPlay::CustomBackend::KAR_GameMode::Top_Ride)));
  windowLayout->addWidget(gameModeDropDown, 3, 1, 1, -1);

  // loads the ROMs and make them options of game flavor
  gameFlavorDropDown = new QComboBox();

  QStringList ROMPaths = Settings::Instance().GetPaths();
  for (uint32 i = 0; i < ROMPaths.size(); ++i)
  {
    // split paths and extract the file name
    std::filesystem::path p(ROMPaths[i].toStdString());
    gameFlavorDropDown->addItem(QString(tr(p.filename().string().c_str())));
  }
  windowLayout->addWidget(gameFlavorDropDown, 3, 2, 1, -1);

  //adds max player count
  maxPLayerLabel = new QLabel();
  maxPLayerLabel->setText(tr("Max Players: "));
  windowLayout->addWidget(maxPLayerLabel, 4, 0, 1, -1);

  maxPlayerInputFeild = new QLineEdit();
  maxPlayerInputFeild->setText(QString::fromStdString(std::to_string(Config::CONFIG_SETTING_LOBBY_MAX_PLAYER_COUNT)));
  windowLayout->addWidget(maxPlayerInputFeild, 4, 1, 1, Qt::AlignRight);

  //adds port to use
  portLabel = new QLabel();
  portLabel->setText(tr("Port: "));
  windowLayout->addWidget(portLabel, 5, 0, 1, -1);

  portInputFeild = new QLineEdit();
  portInputFeild->setText(QString::fromStdString(std::to_string(Config::CONFIG_SETTING_LOBBY_HOST_PORT)));
  windowLayout->addWidget(portInputFeild, 5, 1, 1, Qt::AlignRight);

  lobbyPasswordCheckbox = new QCheckBox();
  lobbyPasswordCheckbox->setChecked(false);
  lobbyPasswordCheckbox->setText(tr("Use Password"));
  windowLayout->addWidget(lobbyPasswordCheckbox, 6, 0, 1, -1);

  // adds password
  lobbyPasswordInputFeild = new QLineEdit();
  lobbyPasswordInputFeild->setPlaceholderText(tr("password...."));
  windowLayout->addWidget(lobbyPasswordInputFeild, 7, 0, 1, -1);

  //add mod list
  modList = new QListWidget;
  windowLayout->addWidget(modList, 8, 0, 1, -1);
  modList->addItem(QString(tr("Gekko Code Mod")));
  modList->addItem(QString(tr("Texture Mod")));
  modList->addItem(QString(tr("Model Mod")));

  //add start hosting button
  startHostingGameButton = new QPushButton;
  windowLayout->addWidget(startHostingGameButton, 9, 0, 1, -1);
  startHostingGameButton->setText(tr("Start Game"));

  //finalizes the layout for the GUI
  setLayout(windowLayout);

//  m_main_layout = new QGridLayout;
//  m_button_box = new QDialogButtonBox(QDialogButtonBox::Cancel);
//  m_nickname_edit = new QLineEdit;
// // m_connection_type = new QComboBox;
//
//  //sets the text for the lobby refresh button
//  refreshLobbyListButton = new NonDefaultQPushButton(tr("Refresh Lobbies"));
//
//
//  m_tab_widget = new QTabWidget;
//
//  m_nickname_edit->setValidator(
//      new UTF8CodePointCountValidator(NetPlay::MAX_NAME_LENGTH, m_nickname_edit));
//
//  // defines the widget menu for list of Lobbies and finding a game
//  auto* connection_widget = new QWidget;
//  auto* connection_layout = new QGridLayout;
//
//  m_connect_button = new NonDefaultQPushButton(tr("Connect"));
//  //lobbyList = new QListWidget;
//  //connection_layout->addWidget(lobbyList, 2, 0, 1, -1);
//  //lobbyList->addItem()
//  //m_ip_label = new QLabel;
//  //m_ip_edit = new QLineEdit;
//  //m_connect_port_label = new QLabel(tr("Port:"));
//  //m_connect_port_box = new QSpinBox;
//  
//
// // m_connect_port_box->setMaximum(65535);
//
//  //connection_layout->addWidget(m_ip_label, 0, 0);
//  //connection_layout->addWidget(m_ip_edit, 0, 1);
//  //connection_layout->addWidget(m_connect_port_label, 0, 2);
//  //connection_layout->addWidget(m_connect_port_box, 0, 3);
//  auto* const alert_label = new QLabel(
//      tr("ALERT:\n\n"
//         "----KARphin is a experimental fork of Dolphin. Use at your own risk.----\n\n"
//         "All players must use the same KARphin version. KARphin Lobbies will show if they use mods. Some mods can cause lag, take that into account.\n"
//         "If enabled, SD cards must be identical between players.\n"
//         "If DSP LLE is used, DSP ROMs must be identical between players.\n"
//         "If a game is hanging on boot, it may not support Dual Core Netplay."
//         " Disable Dual Core.\n"));
//
//  // Prevent the label from stretching vertically so the spacer gets all the extra space
//  alert_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
//
//  connection_layout->addWidget(alert_label, 1, 0, 1, -1);
//  connection_layout->addItem(new QSpacerItem(1, 1), 2, 0, -1, -1);
//  connection_layout->addWidget(m_connect_button, 3, 3, Qt::AlignRight);
//
//  connection_widget->setLayout(connection_layout);
//
//  // defines the widget menu for making a Lobby
//  auto* host_widget = new QWidget;
//  auto* host_layout = new QGridLayout;
//  m_host_port_label = new QLabel(tr("Port:"));
//  m_host_port_box = new QSpinBox;
//  m_host_force_port_check = new QCheckBox(tr("Force Listen Port:"));
//  m_host_force_port_box = new QSpinBox;
//  m_host_chunked_upload_limit_check = new QCheckBox(tr("Limit Chunked Upload Speed:"));
//  m_host_chunked_upload_limit_box = new QSpinBox;
//  m_host_server_browser = new QCheckBox(tr("Show in server browser"));
//  m_host_server_name = new QLineEdit;
//  m_host_server_password = new QLineEdit;
//  m_host_server_region = new QComboBox;
//
////#ifdef USE_UPNP
////  m_host_upnp = new QCheckBox(tr("Forward port (UPnP)"));
////#endif
//  m_host_games = new QListWidget;
//  m_host_button = new NonDefaultQPushButton(tr("Host"));
//
//  m_host_port_box->setMaximum(65535);
//  m_host_force_port_box->setMaximum(65535);
//  m_host_chunked_upload_limit_box->setRange(1, 1000000);
//  m_host_chunked_upload_limit_box->setSingleStep(100);
//  m_host_chunked_upload_limit_box->setSuffix(QStringLiteral(" kbps"));
//
//  m_host_chunked_upload_limit_check->setToolTip(tr(
//      "This will limit the speed of chunked uploading per client, which is used for save sync."));
//
//  m_host_server_name->setToolTip(tr("Name of your session shown in the server browser"));
//  m_host_server_name->setPlaceholderText(tr("Name"));
//  m_host_server_password->setToolTip(tr("Password for joining your game (leave empty for none)"));
//  m_host_server_password->setPlaceholderText(tr("Password"));
//
//  for (const auto& region : NetPlayIndex::GetRegions())
//  {
//    m_host_server_region->addItem(
//        tr("%1 (%2)").arg(tr(region.second.c_str())).arg(QString::fromStdString(region.first)),
//        QString::fromStdString(region.first));
//  }
//
//  host_layout->addWidget(m_host_port_label, 0, 0);
//  host_layout->addWidget(m_host_port_box, 0, 1);
////#ifdef USE_UPNP
////  host_layout->addWidget(m_host_upnp, 0, 2);
////#endif
//  host_layout->addWidget(m_host_server_browser, 1, 0);
//  host_layout->addWidget(m_host_server_region, 1, 1);
//  host_layout->addWidget(m_host_server_name, 1, 2);
//  host_layout->addWidget(m_host_server_password, 1, 3);
//  host_layout->addWidget(m_host_games, 2, 0, 1, -1);
//  host_layout->addWidget(m_host_force_port_check, 3, 0);
//  host_layout->addWidget(m_host_force_port_box, 3, 1, Qt::AlignLeft);
//  host_layout->addWidget(m_host_chunked_upload_limit_check, 4, 0);
//  host_layout->addWidget(m_host_chunked_upload_limit_box, 4, 1, Qt::AlignLeft);
//  host_layout->addWidget(m_host_button, 4, 3, 2, 1, Qt::AlignRight);
//
//  host_widget->setLayout(host_layout);
//
//  //m_connection_type->addItem(tr("Direct Connection"));
//  //m_connection_type->addItem(tr("Traversal Server"));
//
//  //m_main_layout->addWidget(new QLabel(tr("Connection Type:")), 0, 0);
// // m_main_layout->addWidget(m_connection_type, 0, 1);
//  m_main_layout->addWidget(refreshLobbyListButton, 0, 2);
//  m_main_layout->addWidget(new QLabel(tr("Nickname:")), 1, 0);
//  m_main_layout->addWidget(m_nickname_edit, 1, 1);
//  m_main_layout->addWidget(m_tab_widget, 2, 0, 1, -1);
//  m_main_layout->addWidget(m_button_box, 3, 0, 1, -1);
//
//  // Tabs
//  m_tab_widget->addTab(connection_widget, tr("Lobbies"));
//  m_tab_widget->addTab(host_widget, tr("Create Lobby"));
//
//  setLayout(m_main_layout);
}

void NetPlaySetupDialog::ConnectWidgets()
{
  //general widgets

  //lobby list widgets
 // connect(refreshLobbyListButton, &QPushButton::clicked, this, &NetPlaySetupDialog::RefreshLobbies);
 // connect(joinGameButton, &QPushButton::clicked, this, &QDialog::accept);

  //lobby maker widgets
  connect(startHostingGameButton, &QPushButton::clicked, this, &QDialog::accept);

// // connect(m_connection_type, &QComboBox::currentIndexChanged, this,
//  //        &NetPlaySetupDialog::OnConnectionTypeChanged);
//  connect(m_nickname_edit, &QLineEdit::textChanged, this, &NetPlaySetupDialog::SaveSettings);
//
//  // Connect widget
//  //connect(m_ip_edit, &QLineEdit::textChanged, this, &NetPlaySetupDialog::SaveSettings);
//  //connect(m_connect_port_box, &QSpinBox::valueChanged, this, &NetPlaySetupDialog::SaveSettings);
//
//  // Host widget
//  connect(m_host_port_box, &QSpinBox::valueChanged, this, &NetPlaySetupDialog::SaveSettings);
//  connect(m_host_games, &QListWidget::currentRowChanged, [this](int index) {
//    Settings::GetQSettings().setValue(QStringLiteral("netplay/hostgame"),
//                                      m_host_games->item(index)->text());
//  });
//
//  connect(m_host_games, &QListWidget::itemDoubleClicked, this, &NetPlaySetupDialog::accept);
//
//  connect(m_host_force_port_check, &QCheckBox::toggled,
//          [this](bool value) { m_host_force_port_box->setEnabled(value); });
//  connect(m_host_chunked_upload_limit_check, &QCheckBox::toggled, this, [this](bool value) {
//    m_host_chunked_upload_limit_box->setEnabled(value);
//    SaveSettings();
//  });
//  connect(m_host_chunked_upload_limit_box, &QSpinBox::valueChanged, this,
//          &NetPlaySetupDialog::SaveSettings);
//
//  connect(m_host_server_browser, &QCheckBox::toggled, this, &NetPlaySetupDialog::SaveSettings);
//  connect(m_host_server_name, &QLineEdit::textChanged, this, &NetPlaySetupDialog::SaveSettings);
//  connect(m_host_server_password, &QLineEdit::textChanged, this, &NetPlaySetupDialog::SaveSettings);
//  connect(m_host_server_region,
//          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
//          &NetPlaySetupDialog::SaveSettings);
//
////#ifdef USE_UPNP
////  connect(m_host_upnp, &QCheckBox::stateChanged, this, &NetPlaySetupDialog::SaveSettings);
////#endif
//
//  connect(m_connect_button, &QPushButton::clicked, this, &QDialog::accept);
//  connect(m_host_button, &QPushButton::clicked, this, &QDialog::accept);
//  connect(m_button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
//  connect(refreshLobbyListButton, &QPushButton::clicked, this,
//          &NetPlaySetupDialog::GetLobbies);
//  connect(m_host_server_browser, &QCheckBox::toggled, this, [this](bool value) {
//    m_host_server_region->setEnabled(value);
//    m_host_server_name->setEnabled(value);
//    m_host_server_password->setEnabled(value);
//  });
}

void NetPlaySetupDialog::SaveSettings()
{
  //Config::SetBaseOrCurrent(Config::NETPLAY_NICKNAME,
  Config::CONFIG_SETTING_USER_NICKNAME = onlineNickNameInputFeild->text().toStdString();

  Config::CONFIG_SETTING_LOBBY_NAME = lobbyNameInputFeild->text().toStdString();
  Config::CONFIG_SETTING_LOBBY_DESC = descriptionInputFeild->text().toStdString();

  Config::CONFIG_SETTING_LOBBY_MAX_PLAYER_COUNT = maxPlayerInputFeild->text().toUInt();

  Config::CONFIG_SETTING_LOBBY_HOST_PORT = portInputFeild->text().toUInt();

  Config::CONFIG_SETTING_LOBBY_PASSWORD = lobbyPasswordInputFeild->text().toStdString();
  Config::CONFIG_SETTING_LOBBY_PASSWORD_IS_ENABLED = lobbyPasswordCheckbox->isChecked();

  Config::CONFIG_SETTING_GAME_MODE = (NetPlay::CustomBackend::KAR_GameMode)(u8)gameModeDropDown->currentIndex();
  Config::CONFIG_SETTING_GAME_CATAGORY = (NetPlay::CustomBackend::KAR_GameCatagory)(u8)gameCatagoryDropDown->currentIndex();
  Config::CONFIG_SETTING_GAME_ROM_NAME = gameFlavorDropDown->currentText().toStdString();
}

void NetPlaySetupDialog::show()
{
  // hyjack the update to call steam callbacks
  //SteamAPI_RunCallbacks();

  // rebuilds the list of lobbies
  //RebuildLobbyList();

  QDialog::show();
}

void NetPlaySetupDialog::accept()
{
  //the player is always hosting in this menu

  //checks if the name is too long
  if (onlineNickNameInputFeild->text().toStdString().size() > NETPLAY_LIMIT_NAME_LENGTH)
  {
    ModalMessageBox::information(this, tr("Net Play Error"),
                                 tr("Chosen online nickname is longer then the allowed length!"));
    return;
  }

  // saves all settings
  SaveSettings();

  // gets the selected ROM to play
  const char* gameFlavorStr = gameFlavorDropDown->currentText().toStdString().c_str();
  UICommon::GameFile ROMToPlay;
  QStringList ROMPaths = Settings::Instance().GetPaths();
  for (uint32 i = 0; i < ROMPaths.size(); ++i)
  {
    std::filesystem::path p(ROMPaths[i].toStdString());
    if (!strcmp(p.filename().string().c_str(), gameFlavorStr))
    {
      ROMToPlay = UICommon::GameFile(ROMPaths[i].toStdString());
      break;
    }
  }

  // pass the chosen game into the Host
  emit Host(ROMToPlay);

  //if it's in the lobby maker menu || start a lobby
  //if (isInLobbyMakerMenu)
  //{
  // SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, 4);
  //
  //KARLobby* lobby = &lobbies.emplace_back(KARLobby());
  // lobby->lobbyName = "Test";
  //lobbyListIsDirty = true;
  //}

  

  //retrives any mods

  //if the Player is in the lobby menu, we can assume they are attempting to join a lobby
  //if (windowTabLayout->currentIndex() == 0)
  //{
  //  //get the selected lobby
  //  //QListWidgetItem* selectedLobby = lobbyList->currentItem();
  //
  //  //get the IP and Room ID from the lobby to connect to
  //  Config::SetBaseOrCurrent(Config::NETPLAY_HOST_CODE, SteamMatchmaking()->GetLobbyData(lobbies[lobbyList->currentIndex().row()].lobbyID, "KAR_HostCode"));
  //
  //  //set the IP and room code to connect to
  //
  //  emit Join();
  //}

  //if the PLayer is in the build a lobby menu, we can assume they are attempting to host a game
 // else
 // {
    
  //}
}

//void NetPlaySetupDialog::PopulateGameList()
//{
 // QSignalBlocker blocker(m_host_games);

  //render a list of lobbies

 // auto* item = new QListWidgetItem(QString::fromStdString("OwO"));
  //item->setData(Qt::UserRole, QVariant::fromValue(std::move(game)));
 // m_host_games->addItem(item);

  //m_host_games->clear();
  //for (int i = 0; i < m_game_list_model.rowCount(QModelIndex()); i++)
  //{
  //  std::shared_ptr<const UICommon::GameFile> game = m_game_list_model.GetGameFile(i);
  //
  //  auto* item =
  //      new QListWidgetItem(QString::fromStdString(m_game_list_model.GetNetPlayName(*game)));
  //  item->setData(Qt::UserRole, QVariant::fromValue(std::move(game)));
  //  m_host_games->addItem(item);
  //}
  //
  //m_host_games->sortItems();
  //
  //const QString selected_game =
  //    Settings::GetQSettings().value(QStringLiteral("netplay/hostgame"), QString{}).toString();
  //const auto find_list = m_host_games->findItems(selected_game, Qt::MatchFlag::MatchExactly);
  //
  //if (find_list.count() > 0)
  //  m_host_games->setCurrentItem(find_list[0]);
//}

//void NetPlaySetupDialog::ResetTraversalHost()
//{
//  Config::SetBaseOrCurrent(Config::NETPLAY_TRAVERSAL_SERVER,
//                           Config::NETPLAY_TRAVERSAL_SERVER.GetDefaultValue());
//  Config::SetBaseOrCurrent(Config::NETPLAY_TRAVERSAL_PORT,
//                           Config::NETPLAY_TRAVERSAL_PORT.GetDefaultValue());
//
//  ModalMessageBox::information(
//      this, tr("Reset Traversal Server"),
//      tr("Reset Traversal Server to %1:%2")
//          .arg(QString::fromStdString(Config::NETPLAY_TRAVERSAL_SERVER.GetDefaultValue()),
//               QString::number(Config::NETPLAY_TRAVERSAL_PORT.GetDefaultValue())));
//}
