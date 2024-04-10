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

#include <Core/CustomNetplayBackend/KARLobby.hpp>

NetPlaySetupDialog::NetPlaySetupDialog(const GameListModel& game_list_model, QWidget* parent)
    : QDialog(parent), m_game_list_model(game_list_model)
{
  setWindowTitle(tr("KARphin NetPlay Host Setup"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  CreateMainLayout();

  ConnectWidgets();
}

void NetPlaySetupDialog::CreateMainLayout()
{
  m_main_layout = new QGridLayout;

  m_nickname_edit = new QLineEdit;
  m_nickname_edit->setValidator(
      new UTF8CodePointCountValidator(NetPlay::MAX_NAME_LENGTH, m_nickname_edit));

  m_host_server_name = new QLineEdit;
  m_host_server_name->setToolTip(tr("Name of your session shown in the server browser"));
  m_host_server_name->setPlaceholderText(tr("Name..."));

  m_host_server_password = new QLineEdit;
  m_host_server_password->setToolTip(tr("Password for joining your game (leave empty for none)"));
  m_host_server_password->setPlaceholderText(tr("Password..."));
  m_host_server_password->setEnabled(false);

  m_host_server_hasPassword = new QCheckBox();
  m_host_server_hasPassword->setText(tr("Password?"));

  m_host_server_region = new QComboBox;
  for (const auto& region : NetPlayIndex::GetRegions())
  {
    m_host_server_region->addItem(
        tr("%1 (%2)").arg(tr(region.second.c_str())).arg(QString::fromStdString(region.first)),
        QString::fromStdString(region.first));
  }

  m_host_server_catagory = new QComboBox;
  for (uint32 i = 0; i < (uint32)NetPlay::CustomBackend::KAR::GameCatagory::Count; ++i)
    m_host_server_catagory->addItem(tr(NetPlay::CustomBackend::KAR::GameCatagoryToStr(
        (NetPlay::CustomBackend::KAR::GameCatagory)i)));

  m_host_server_game_mode = new QComboBox;
  for (uint32 i = 0; i < (uint32)NetPlay::CustomBackend::KAR::GameMode::Count; ++i)
    m_host_server_game_mode->addItem(tr(NetPlay::CustomBackend::KAR::GameModeToStr((NetPlay::CustomBackend::KAR::GameMode)i)));

  m_button_cancel = new QDialogButtonBox(QDialogButtonBox::Cancel);
  
  m_host_chunked_upload_limit_check = new QCheckBox(tr("Limit Chunked Upload Speed:"));
  m_host_chunked_upload_limit_box = new QSpinBox;
  m_host_chunked_upload_limit_box->setEnabled(false);

  m_ROM_list = new QListWidget;
  m_host_button = new NonDefaultQPushButton(tr("Host"));

  m_host_chunked_upload_limit_box->setRange(1, 1000000);
  m_host_chunked_upload_limit_box->setSingleStep(100);
  m_host_chunked_upload_limit_box->setSuffix(QStringLiteral(" kbps"));

  m_host_chunked_upload_limit_check->setToolTip(tr(
      "This will limit the speed of chunked uploading per client, which is used for save sync."));

  //adds to the GUI
  m_main_layout->addWidget(new QLabel(tr("Nickname:")), 0, 0);
  m_main_layout->addWidget(m_nickname_edit, 0, 1);
  m_main_layout->addWidget(new QLabel(tr("Lobby Name:")), 1, 0);
  m_main_layout->addWidget(m_host_server_name, 1, 1);
  m_main_layout->addWidget(m_host_server_hasPassword, 2, 0);
  m_main_layout->addWidget(m_host_server_password, 2, 1);

  m_main_layout->addWidget(m_host_server_region, 3, 1);
  m_main_layout->addWidget(m_host_server_catagory, 3, 2);
  m_main_layout->addWidget(m_host_server_game_mode, 3, 3);
  
  m_main_layout->addWidget(m_ROM_list, 4, 0, 1, -1);
  m_main_layout->addWidget(m_host_chunked_upload_limit_check, 6, 0);
  m_main_layout->addWidget(m_host_chunked_upload_limit_box, 6, 1, Qt::AlignLeft);
  

  m_main_layout->addWidget(m_host_button, 6, 3, 2, 1, Qt::AlignRight);
  m_main_layout->addWidget(m_button_cancel, 9, 0, 1, -1);

  setLayout(m_main_layout);
}

void NetPlaySetupDialog::ConnectWidgets()
{
  connect(m_nickname_edit, &QLineEdit::textChanged, this, &NetPlaySetupDialog::SaveSettings);

  connect(m_ROM_list, &QListWidget::currentRowChanged, [this](int index) {
    Settings::GetQSettings().setValue(QStringLiteral("netplay/hostgame"),
                                      m_ROM_list->item(index)->text());
  });

  connect(m_ROM_list, &QListWidget::itemDoubleClicked, this, &NetPlaySetupDialog::accept);

  connect(m_host_chunked_upload_limit_check, &QCheckBox::toggled, this, [this](bool value) {
    m_host_chunked_upload_limit_box->setEnabled(value);
    SaveSettings();
  });
  connect(m_host_chunked_upload_limit_box, &QSpinBox::valueChanged, this,
          &NetPlaySetupDialog::SaveSettings);

  connect(m_host_server_name, &QLineEdit::textChanged, this, &NetPlaySetupDialog::SaveSettings);

  connect(m_host_server_password, &QLineEdit::textChanged, this, &NetPlaySetupDialog::SaveSettings);
  connect(m_host_server_hasPassword, &QCheckBox::toggled, this, [this](bool value) {
    m_host_server_password->setEnabled(value);
    SaveSettings();
  });

  connect(m_host_server_region,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
          &NetPlaySetupDialog::SaveSettings);

  connect(m_host_button, &QPushButton::clicked, this, &QDialog::accept);
  connect(m_button_cancel, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void NetPlaySetupDialog::SaveSettings()
{
  Config::ConfigChangeCallbackGuard config_guard;

  Config::NETPLAY_USER_NICKNAME =
      m_nickname_edit->text().toStdString();
#ifdef USE_UPNP
  Config::NETPLAY_LOBBY_USE_UPNP =
      true;
#endif

  Config::NETPLAY_ENABLE_CHUNKED_UPLOAD_LIMIT =
      m_host_chunked_upload_limit_check
          ->isChecked();
  Config::NETPLAY_CHUNKED_UPLOAD_LIMIT = m_host_chunked_upload_limit_box->value();
  Config::NETPLAY_LOBBY_REGION = m_host_server_region->currentData().toString().toStdString();
  Config::NETPLAY_LOBBY_NAME = m_host_server_name->text().toStdString();
  Config::NETPLAY_LOBBY_PASSWORD = m_host_server_password->text().toStdString();
}

void NetPlaySetupDialog::show()
{
  PopulateGameList();
  QDialog::show();
}

void NetPlaySetupDialog::accept()
{
  SaveSettings();

    auto items = m_ROM_list->selectedItems();
    if (items.empty())
    {
      ModalMessageBox::critical(this, tr("Error"), tr("You must select a game to host!"));
      return;
    }

    if (m_host_server_name->text().isEmpty())
    {
      ModalMessageBox::critical(this, tr("Error"), tr("You must provide a name for your session!"));
      return;
    }

    if (m_host_server_region->currentData().toString().isEmpty())
    {
      ModalMessageBox::critical(this, tr("Error"),
                                tr("You must provide a region for your session!"));
      return;
    }

    emit Host(*items[0]->data(Qt::UserRole).value<std::shared_ptr<const UICommon::GameFile>>());
}

void NetPlaySetupDialog::PopulateGameList()
{
  QSignalBlocker blocker(m_ROM_list);

  m_ROM_list->clear();
  for (int i = 0; i < m_game_list_model.rowCount(QModelIndex()); i++)
  {
    std::shared_ptr<const UICommon::GameFile> game = m_game_list_model.GetGameFile(i);

    auto* item =
        new QListWidgetItem(QString::fromStdString(m_game_list_model.GetNetPlayName(*game)));
    item->setData(Qt::UserRole, QVariant::fromValue(std::move(game)));
    m_ROM_list->addItem(item);
  }

  m_ROM_list->sortItems();

  const QString selected_game =
      Settings::GetQSettings().value(QStringLiteral("netplay/hostgame"), QString{}).toString();
  const auto find_list = m_ROM_list->findItems(selected_game, Qt::MatchFlag::MatchExactly);

  if (find_list.count() > 0)
    m_ROM_list->setCurrentItem(find_list[0]);
}
