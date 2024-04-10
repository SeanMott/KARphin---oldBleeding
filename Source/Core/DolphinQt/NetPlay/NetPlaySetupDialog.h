// Copyright 2017 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDialog>

#include "DolphinQt/GameList/GameListModel.h"

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

class NetPlaySetupDialog : public QDialog
{
  Q_OBJECT
public:
  explicit NetPlaySetupDialog(const GameListModel& game_list_model, QWidget* parent);

  void accept() override;
  void show();

signals:
  bool Join();
  bool Host(const UICommon::GameFile& game);

private:
  void CreateMainLayout();
  void ConnectWidgets();
  void PopulateGameList();

  void SaveSettings();

  QLineEdit* m_host_server_name;
  QLineEdit* m_host_server_password;
  QCheckBox* m_host_server_hasPassword;
  QComboBox* m_host_server_region;
  QComboBox* m_host_server_catagory;
  QComboBox* m_host_server_game_mode;

  //lobby description

  //max players

  //should spectators be allowed

  //max spectators

  QListWidget* m_ROM_list;

  QLineEdit* m_nickname_edit;
  QGridLayout* m_main_layout;

  QDialogButtonBox* m_button_cancel;
  QPushButton* m_host_button;

  QCheckBox* m_host_chunked_upload_limit_check;
  QSpinBox* m_host_chunked_upload_limit_box;

  const GameListModel& m_game_list_model;
};
