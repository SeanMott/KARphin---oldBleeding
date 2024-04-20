// Copyright 2015 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DolphinQt/MainWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QIcon>
#include <QMimeData>
#include <QStackedWidget>
#include <QStyleHints>
#include <QVBoxLayout>
#include <QWindow>

#include <fmt/format.h>

#include <future>
#include <optional>
#include <variant>

#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
#include <signal.h>

#include "QtUtils/SignalDaemon.h"
#endif

#ifndef _WIN32
#include <qpa/qplatformnativeinterface.h>
#endif

#include "Common/ScopeGuard.h"
#include "Common/Version.h"
#include "Common/WindowSystemInfo.h"

#include "Core/AchievementManager.h"
#include "Core/Boot/Boot.h"
#include "Core/BootManager.h"
#include "Core/CommonTitles.h"
#include "Core/Config/AchievementSettings.h"
#include "Core/Config/MainSettings.h"
#include "Core/Config/NetplaySettings.h"
#include "Core/Config/UISettings.h"
#include "Core/Config/WiimoteSettings.h"
#include "Core/Core.h"
#include "Core/FreeLookManager.h"
#include "Core/HW/DVD/DVDInterface.h"
#include "Core/HW/GBAPad.h"
#include "Core/HW/GCKeyboard.h"
#include "Core/HW/GCPad.h"
#include "Core/HW/ProcessorInterface.h"
#include "Core/HW/SI/SI_Device.h"
#include "Core/HW/Wiimote.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"
#include "Core/HotkeyManager.h"
#include "Core/IOS/USB/Bluetooth/BTEmu.h"
#include "Core/IOS/USB/Bluetooth/WiimoteDevice.h"
#include "Core/Movie.h"
#include "Core/NetPlayClient.h"
#include "Core/NetPlayProto.h"
#include "Core/NetPlayServer.h"
#include "Core/State.h"
#include "Core/System.h"
#include "Core/WiiUtils.h"

#include "DiscIO/DirectoryBlob.h"
#include "DiscIO/NANDImporter.h"
#include "DiscIO/RiivolutionPatcher.h"

#include "DolphinQt/AboutDialog.h"
#include "DolphinQt/Achievements/AchievementsWindow.h"
#include "DolphinQt/CheatsManager.h"
#include "DolphinQt/Config/ControllersWindow.h"
#include "DolphinQt/Config/FreeLookWindow.h"
#include "DolphinQt/Config/Graphics/GraphicsWindow.h"
#include "DolphinQt/Config/LogConfigWidget.h"
#include "DolphinQt/Config/LogWidget.h"
#include "DolphinQt/Config/Mapping/MappingWindow.h"
#include "DolphinQt/Config/SettingsWindow.h"
#include "DolphinQt/Debugger/AssemblerWidget.h"
#include "DolphinQt/Debugger/BreakpointWidget.h"
#include "DolphinQt/Debugger/CodeViewWidget.h"
#include "DolphinQt/Debugger/CodeWidget.h"
#include "DolphinQt/Debugger/JITWidget.h"
#include "DolphinQt/Debugger/MemoryWidget.h"
#include "DolphinQt/Debugger/NetworkWidget.h"
#include "DolphinQt/Debugger/RegisterWidget.h"
#include "DolphinQt/Debugger/ThreadWidget.h"
#include "DolphinQt/Debugger/WatchWidget.h"
#include "DolphinQt/DiscordHandler.h"
#include "DolphinQt/FIFO/FIFOPlayerWindow.h"
#include "DolphinQt/GCMemcardManager.h"
#include "DolphinQt/GameList/GameList.h"
#include "DolphinQt/Host.h"
#include "DolphinQt/HotkeyScheduler.h"
#include "DolphinQt/InfinityBase/InfinityBaseWindow.h"
#include "DolphinQt/MenuBar.h"
#include "DolphinQt/NKitWarningDialog.h"
#include "DolphinQt/NetPlay/NetPlayBrowser.h"
#include "DolphinQt/NetPlay/NetPlayDialog.h"
#include "DolphinQt/NetPlay/NetPlaySetupDialog.h"
#include "DolphinQt/QtUtils/DolphinFileDialog.h"
#include "DolphinQt/QtUtils/FileOpenEventFilter.h"
#include "DolphinQt/QtUtils/ModalMessageBox.h"
#include "DolphinQt/QtUtils/ParallelProgressDialog.h"
#include "DolphinQt/QtUtils/QueueOnObject.h"
#include "DolphinQt/QtUtils/RunOnObject.h"
#include "DolphinQt/QtUtils/SetWindowDecorations.h"
#include "DolphinQt/QtUtils/WindowActivationEventFilter.h"
#include "DolphinQt/RenderWidget.h"
#include "DolphinQt/ResourcePackManager.h"
#include "DolphinQt/Resources.h"
#include "DolphinQt/RiivolutionBootWidget.h"
#include "DolphinQt/SearchBar.h"
#include "DolphinQt/Settings.h"
#include "DolphinQt/SkylanderPortal/SkylanderPortalWindow.h"
#include "DolphinQt/TAS/GBATASInputWindow.h"
#include "DolphinQt/TAS/GCTASInputWindow.h"
#include "DolphinQt/TAS/WiiTASInputWindow.h"
#include "DolphinQt/ToolBar.h"
#include "DolphinQt/WiiUpdate.h"

#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/GCAdapter.h"

#include "UICommon/DiscordPresence.h"
#include "UICommon/GameFile.h"
#include "UICommon/ResourcePack/Manager.h"
#include "UICommon/ResourcePack/Manifest.h"
#include "UICommon/ResourcePack/ResourcePack.h"

#include "UICommon/UICommon.h"

#include "VideoCommon/NetPlayChatUI.h"
#include "VideoCommon/VideoConfig.h"

#ifdef HAVE_XRANDR
#include "UICommon/X11Utils.h"
// This #define within X11/X.h conflicts with our WiimoteSource enum.
#undef None
#endif

#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
void MainWindow::OnSignal()
{
  close();
}

static void InstallSignalHandler()
{
  struct sigaction sa;
  sa.sa_handler = &SignalDaemon::HandleInterrupt;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESETHAND;
  sigaction(SIGINT, &sa, nullptr);
  sigaction(SIGTERM, &sa, nullptr);
}
#endif

static WindowSystemType GetWindowSystemType()
{
  // Determine WSI type based on Qt platform.
  QString platform_name = QGuiApplication::platformName();
  if (platform_name == QStringLiteral("windows"))
    return WindowSystemType::Windows;
  else if (platform_name == QStringLiteral("cocoa"))
    return WindowSystemType::MacOS;
  else if (platform_name == QStringLiteral("xcb"))
    return WindowSystemType::X11;
  else if (platform_name == QStringLiteral("wayland"))
    return WindowSystemType::Wayland;
  else if (platform_name == QStringLiteral("haiku"))
    return WindowSystemType::Haiku;

  ModalMessageBox::critical(
      nullptr, QStringLiteral("Error"),
      QString::asprintf("Unknown Qt platform: %s", platform_name.toStdString().c_str()));
  return WindowSystemType::Headless;
}

static WindowSystemInfo GetWindowSystemInfo(QWindow* window)
{
  WindowSystemInfo wsi;
  wsi.type = GetWindowSystemType();

  // Our Win32 Qt external doesn't have the private API.
#if defined(WIN32) || defined(__APPLE__) || defined(__HAIKU__)
  wsi.render_window = window ? reinterpret_cast<void*>(window->winId()) : nullptr;
  wsi.render_surface = wsi.render_window;
#else
  QPlatformNativeInterface* pni = QGuiApplication::platformNativeInterface();
  wsi.display_connection = pni->nativeResourceForWindow("display", window);
  if (wsi.type == WindowSystemType::Wayland)
    wsi.render_window = window ? pni->nativeResourceForWindow("surface", window) : nullptr;
  else
    wsi.render_window = window ? reinterpret_cast<void*>(window->winId()) : nullptr;
  wsi.render_surface = wsi.render_window;
#endif
  wsi.render_surface_scale = window ? static_cast<float>(window->devicePixelRatio()) : 1.0f;

  return wsi;
}

static std::vector<std::string> StringListToStdVector(QStringList list)
{
  std::vector<std::string> result;
  result.reserve(list.size());

  for (const QString& s : list)
    result.push_back(s.toStdString());

  return result;
}

MainWindow::MainWindow(std::unique_ptr<BootParameters> boot_parameters,
                       const std::string& movie_path)
    : QMainWindow(nullptr)
{
  setWindowTitle(QString::fromStdString(Common::GetScmRevStr()));
  setWindowIcon(Resources::GetAppIcon());
  setUnifiedTitleAndToolBarOnMac(true);
  setAcceptDrops(true);
  setAttribute(Qt::WA_NativeWindow);

  InitControllers();

  CreateComponents();

  ConnectGameList();
  ConnectHost();
  ConnectToolBar();
  ConnectRenderWidget();
  ConnectStack();
  ConnectMenuBar();
  ConnectHotkeys();

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
          [](Qt::ColorScheme colorScheme) { Settings::Instance().ApplyStyle(); });
#endif

  connect(m_cheats_manager, &CheatsManager::OpenGeneralSettings, this,
          &MainWindow::ShowGeneralWindow);

//#ifdef USE_RETRO_ACHIEVEMENTS
//  connect(m_cheats_manager, &CheatsManager::OpenAchievementSettings, this,
//          &MainWindow::ShowAchievementSettings);
//  connect(m_game_list, &GameList::OpenAchievementSettings, this,
//          &MainWindow::ShowAchievementSettings);
//#endif  // USE_RETRO_ACHIEVEMENTS

  InitCoreCallbacks();

  NetPlayInit();

#ifdef USE_RETRO_ACHIEVEMENTS
  AchievementManager::GetInstance().Init();
#endif  // USE_RETRO_ACHIEVEMENTS

#if defined(__unix__) || defined(__unix) || defined(__APPLE__)
  auto* daemon = new SignalDaemon(this);

  connect(daemon, &SignalDaemon::InterruptReceived, this, &MainWindow::OnSignal);

  InstallSignalHandler();
#endif

  if (boot_parameters)
  {
    m_pending_boot = std::move(boot_parameters);

    if (!movie_path.empty())
    {
      std::optional<std::string> savestate_path;
      if (Core::System::GetInstance().GetMovie().PlayInput(movie_path, &savestate_path))
      {
        m_pending_boot->boot_session_data.SetSavestateData(std::move(savestate_path),
                                                           DeleteSavestateAfterBoot::No);
        emit RecordingStatusChanged(true);
      }
    }
  }

  m_state_slot =
      std::clamp(Settings::Instance().GetStateSlot(), 1, static_cast<int>(State::NUM_STATES));

  QSettings& settings = Settings::GetQSettings();

  restoreState(settings.value(QStringLiteral("mainwindow/state")).toByteArray());
  restoreGeometry(settings.value(QStringLiteral("mainwindow/geometry")).toByteArray());

  m_render_widget_geometry = settings.value(QStringLiteral("renderwidget/geometry")).toByteArray();

  // Restoring of window states can sometimes go wrong, resulting in widgets being visible when they
  // shouldn't be so we have to reapply all our rules afterwards.
  Settings::Instance().RefreshWidgetVisibility();

  if (!ResourcePack::Init())
  {
    ModalMessageBox::critical(this, tr("Error"),
                              tr("Error occurred while loading some texture packs"));
  }

  for (auto& pack : ResourcePack::GetPacks())
  {
    if (!pack.IsValid())
    {
      ModalMessageBox::critical(this, tr("Error"),
                                tr("Invalid Pack %1 provided: %2")
                                    .arg(QString::fromStdString(pack.GetPath()))
                                    .arg(QString::fromStdString(pack.GetError())));
      return;
    }
  }

  Host::GetInstance()->SetMainWindowHandle(reinterpret_cast<void*>(winId()));
}

MainWindow::~MainWindow()
{
  // Shut down NetPlay first to avoid race condition segfault
  Settings::Instance().ResetNetPlayClient();
  Settings::Instance().ResetNetPlayServer();

#ifdef USE_RETRO_ACHIEVEMENTS
  AchievementManager::GetInstance().Shutdown();
#endif  // USE_RETRO_ACHIEVEMENTS

  delete m_render_widget;
  delete m_netplay_dialog;

  for (int i = 0; i < 4; i++)
  {
    delete m_gc_tas_input_windows[i];
    delete m_gba_tas_input_windows[i];
    delete m_wii_tas_input_windows[i];
  }

  ShutdownControllers();

  QSettings& settings = Settings::GetQSettings();

  settings.setValue(QStringLiteral("mainwindow/state"), saveState());
  settings.setValue(QStringLiteral("mainwindow/geometry"), saveGeometry());

  settings.setValue(QStringLiteral("renderwidget/geometry"), m_render_widget_geometry);

  Config::Save();
}

WindowSystemInfo MainWindow::GetWindowSystemInfo() const
{
  return ::GetWindowSystemInfo(m_render_widget->windowHandle());
}

void MainWindow::InitControllers()
{
  if (g_controller_interface.IsInit())
    return;

  UICommon::InitControllers(::GetWindowSystemInfo(windowHandle()));

  m_hotkey_scheduler = new HotkeyScheduler();
  m_hotkey_scheduler->Start();

  // Defaults won't work reliably without loading and saving the config first

  Wiimote::LoadConfig();
  Wiimote::GetConfig()->SaveConfig();

  Pad::LoadConfig();
  Pad::GetConfig()->SaveConfig();

  Pad::LoadGBAConfig();
  Pad::GetGBAConfig()->SaveConfig();

  Keyboard::LoadConfig();
  Keyboard::GetConfig()->SaveConfig();

  FreeLook::LoadInputConfig();
  FreeLook::GetInputConfig()->SaveConfig();
}

void MainWindow::ShutdownControllers()
{
  m_hotkey_scheduler->Stop();

  Settings::Instance().UnregisterDevicesChangedCallback();

  UICommon::ShutdownControllers();

  m_hotkey_scheduler->deleteLater();
}

void MainWindow::InitCoreCallbacks()
{
  connect(&Settings::Instance(), &Settings::EmulationStateChanged, this, [this](Core::State state) {
    if (state == Core::State::Uninitialized)
      OnStopComplete();

    if (state == Core::State::Running && m_fullscreen_requested)
    {
      FullScreen();
      m_fullscreen_requested = false;
    }
  });
  installEventFilter(this);
  m_render_widget->installEventFilter(this);

  // Handle file open events
  auto* filter = new FileOpenEventFilter(QGuiApplication::instance());
  connect(filter, &FileOpenEventFilter::fileOpened, this, [this](const QString& file_name) {
    StartGame(BootParameters::GenerateFromFile(file_name.toStdString()));
  });
}

static void InstallHotkeyFilter(QWidget* dialog)
{
  auto* filter = new WindowActivationEventFilter(dialog);
  dialog->installEventFilter(filter);

  filter->connect(filter, &WindowActivationEventFilter::windowDeactivated,
                  [] { HotkeyManagerEmu::Enable(true); });
  filter->connect(filter, &WindowActivationEventFilter::windowActivated,
                  [] { HotkeyManagerEmu::Enable(false); });
}

void MainWindow::CreateComponents()
{
  m_menu_bar = new MenuBar(this);
  m_tool_bar = new ToolBar(this);
  m_search_bar = new SearchBar(this);
  m_game_list = new GameList(this);
  m_render_widget = new RenderWidget;
  m_stack = new QStackedWidget(this);

  for (int i = 0; i < 4; i++)
  {
    m_gc_tas_input_windows[i] = new GCTASInputWindow(nullptr, i);
    m_gba_tas_input_windows[i] = new GBATASInputWindow(nullptr, i);
    m_wii_tas_input_windows[i] = new WiiTASInputWindow(nullptr, i);
  }

  m_jit_widget = new JITWidget(this);
  m_log_widget = new LogWidget(this);
  m_log_config_widget = new LogConfigWidget(this);
  m_memory_widget = new MemoryWidget(Core::System::GetInstance(), this);
  m_network_widget = new NetworkWidget(this);
  m_register_widget = new RegisterWidget(this);
  m_thread_widget = new ThreadWidget(this);
  m_watch_widget = new WatchWidget(this);
  m_breakpoint_widget = new BreakpointWidget(this);
  m_code_widget = new CodeWidget(this);
  m_cheats_manager = new CheatsManager(Core::System::GetInstance(), this);
  m_assembler_widget = new AssemblerWidget(this);

  const auto request_watch = [this](QString name, u32 addr) {
    m_watch_widget->AddWatch(name, addr);
  };
  const auto request_breakpoint = [this](u32 addr) { m_breakpoint_widget->AddBP(addr); };
  const auto request_memory_breakpoint = [this](u32 addr) {
    m_breakpoint_widget->AddAddressMBP(addr);
  };
  const auto request_view_in_memory = [this](u32 addr) { m_memory_widget->SetAddress(addr); };
  const auto request_view_in_code = [this](u32 addr) {
    m_code_widget->SetAddress(addr, CodeViewWidget::SetAddressUpdate::WithDetailedUpdate);
  };

  connect(m_watch_widget, &WatchWidget::RequestMemoryBreakpoint, request_memory_breakpoint);
  connect(m_watch_widget, &WatchWidget::ShowMemory, m_memory_widget, &MemoryWidget::SetAddress);
  connect(m_register_widget, &RegisterWidget::RequestMemoryBreakpoint, request_memory_breakpoint);
  connect(m_register_widget, &RegisterWidget::RequestWatch, request_watch);
  connect(m_register_widget, &RegisterWidget::RequestViewInMemory, request_view_in_memory);
  connect(m_register_widget, &RegisterWidget::RequestViewInCode, request_view_in_code);
  connect(m_thread_widget, &ThreadWidget::RequestBreakpoint, request_breakpoint);
  connect(m_thread_widget, &ThreadWidget::RequestMemoryBreakpoint, request_memory_breakpoint);
  connect(m_thread_widget, &ThreadWidget::RequestWatch, request_watch);
  connect(m_thread_widget, &ThreadWidget::RequestViewInMemory, request_view_in_memory);
  connect(m_thread_widget, &ThreadWidget::RequestViewInCode, request_view_in_code);

  connect(m_code_widget, &CodeWidget::BreakpointsChanged, m_breakpoint_widget,
          &BreakpointWidget::Update);
  connect(m_code_widget, &CodeWidget::RequestPPCComparison, m_jit_widget, &JITWidget::Compare);
  connect(m_code_widget, &CodeWidget::ShowMemory, m_memory_widget, &MemoryWidget::SetAddress);
  connect(m_memory_widget, &MemoryWidget::BreakpointsChanged, m_breakpoint_widget,
          &BreakpointWidget::Update);
  connect(m_memory_widget, &MemoryWidget::ShowCode, m_code_widget, [this](u32 address) {
    m_code_widget->SetAddress(address, CodeViewWidget::SetAddressUpdate::WithDetailedUpdate);
  });
  connect(m_memory_widget, &MemoryWidget::RequestWatch, request_watch);

  connect(m_breakpoint_widget, &BreakpointWidget::BreakpointsChanged, m_code_widget,
          &CodeWidget::Update);
  connect(m_breakpoint_widget, &BreakpointWidget::BreakpointsChanged, m_memory_widget,
          &MemoryWidget::Update);
  connect(m_breakpoint_widget, &BreakpointWidget::ShowCode, [this](u32 address) {
    if (Core::GetState() == Core::State::Paused)
      m_code_widget->SetAddress(address, CodeViewWidget::SetAddressUpdate::WithDetailedUpdate);
  });
  connect(m_breakpoint_widget, &BreakpointWidget::ShowMemory, m_memory_widget,
          &MemoryWidget::SetAddress);
  connect(m_cheats_manager, &CheatsManager::ShowMemory, m_memory_widget, &MemoryWidget::SetAddress);
  connect(m_cheats_manager, &CheatsManager::RequestWatch, request_watch);
}

void MainWindow::ConnectMenuBar()
{
  setMenuBar(m_menu_bar);
  // File
  connect(m_menu_bar, &MenuBar::Open, this, &MainWindow::Open);
  connect(m_menu_bar, &MenuBar::Exit, this, &MainWindow::close);
  connect(m_menu_bar, &MenuBar::EjectDisc, this, &MainWindow::EjectDisc);
  connect(m_menu_bar, &MenuBar::ChangeDisc, this, &MainWindow::ChangeDisc);
  connect(m_menu_bar, &MenuBar::PrepInstallForKARNetplay, this, &MainWindow::PrepInstallForKARNetplay);
  connect(m_menu_bar, &MenuBar::SyncKARphinInstanceWithKARWorkshop, this, &MainWindow::SyncKARphinInstanceWithKARWorkshop);

  // Emulation
  connect(m_menu_bar, &MenuBar::Pause, this, &MainWindow::Pause);
  connect(m_menu_bar, &MenuBar::Play, this, [this]() { Play(); });
  connect(m_menu_bar, &MenuBar::Stop, this, &MainWindow::RequestStop);
  connect(m_menu_bar, &MenuBar::Reset, this, &MainWindow::Reset);
  connect(m_menu_bar, &MenuBar::Fullscreen, this, &MainWindow::FullScreen);
  connect(m_menu_bar, &MenuBar::FrameAdvance, this, &MainWindow::FrameAdvance);
  connect(m_menu_bar, &MenuBar::Screenshot, this, &MainWindow::ScreenShot);
  connect(m_menu_bar, &MenuBar::StateLoad, this, &MainWindow::StateLoad);
  connect(m_menu_bar, &MenuBar::StateSave, this, &MainWindow::StateSave);
  connect(m_menu_bar, &MenuBar::StateLoadSlot, this, &MainWindow::StateLoadSlot);
  connect(m_menu_bar, &MenuBar::StateSaveSlot, this, &MainWindow::StateSaveSlot);
  connect(m_menu_bar, &MenuBar::StateLoadSlotAt, this, &MainWindow::StateLoadSlotAt);
  connect(m_menu_bar, &MenuBar::StateSaveSlotAt, this, &MainWindow::StateSaveSlotAt);
  connect(m_menu_bar, &MenuBar::StateLoadUndo, this, &MainWindow::StateLoadUndo);
  connect(m_menu_bar, &MenuBar::StateSaveUndo, this, &MainWindow::StateSaveUndo);
  connect(m_menu_bar, &MenuBar::StateSaveOldest, this, &MainWindow::StateSaveOldest);
  connect(m_menu_bar, &MenuBar::SetStateSlot, this, &MainWindow::SetStateSlot);

  // Options
  connect(m_menu_bar, &MenuBar::Configure, this, &MainWindow::ShowSettingsWindow);
  connect(m_menu_bar, &MenuBar::ConfigureGraphics, this, &MainWindow::ShowGraphicsWindow);
  connect(m_menu_bar, &MenuBar::ConfigureAudio, this, &MainWindow::ShowAudioWindow);
  connect(m_menu_bar, &MenuBar::ConfigureControllers, this, &MainWindow::ShowControllersWindow);
  connect(m_menu_bar, &MenuBar::ConfigureHotkeys, this, &MainWindow::ShowHotkeyDialog);
  connect(m_menu_bar, &MenuBar::ConfigureFreelook, this, &MainWindow::ShowFreeLookWindow);

  // Tools
  connect(m_menu_bar, &MenuBar::ShowMemcardManager, this, &MainWindow::ShowMemcardManager);
  connect(m_menu_bar, &MenuBar::ShowResourcePackManager, this,
          &MainWindow::ShowResourcePackManager);
  connect(m_menu_bar, &MenuBar::ShowCheatsManager, this, &MainWindow::ShowCheatsManager);
  connect(m_menu_bar, &MenuBar::BootGameCubeIPL, this, &MainWindow::OnBootGameCubeIPL);
  connect(m_menu_bar, &MenuBar::ImportNANDBackup, this, &MainWindow::OnImportNANDBackup);
  connect(m_menu_bar, &MenuBar::PerformOnlineUpdate, this, &MainWindow::PerformOnlineUpdate);
  connect(m_menu_bar, &MenuBar::BootWiiSystemMenu, this, &MainWindow::BootWiiSystemMenu);
  connect(m_menu_bar, &MenuBar::StartNetPlay, this, &MainWindow::ShowNetPlaySetupDialog);
  connect(m_menu_bar, &MenuBar::BrowseNetPlay, this, &MainWindow::ShowNetPlayBrowser);
  connect(m_menu_bar, &MenuBar::ShowFIFOPlayer, this, &MainWindow::ShowFIFOPlayer);
  connect(m_menu_bar, &MenuBar::ShowSkylanderPortal, this, &MainWindow::ShowSkylanderPortal);
  connect(m_menu_bar, &MenuBar::ShowInfinityBase, this, &MainWindow::ShowInfinityBase);
  connect(m_menu_bar, &MenuBar::ConnectWiiRemote, this, &MainWindow::OnConnectWiiRemote);

//#ifdef USE_RETRO_ACHIEVEMENTS
//  connect(m_menu_bar, &MenuBar::ShowAchievementsWindow, this, &MainWindow::ShowAchievementsWindow);
//#endif  // USE_RETRO_ACHIEVEMENTS

  // Movie
  connect(m_menu_bar, &MenuBar::PlayRecording, this, &MainWindow::OnPlayRecording);
  connect(m_menu_bar, &MenuBar::StartRecording, this, &MainWindow::OnStartRecording);
  connect(m_menu_bar, &MenuBar::StopRecording, this, &MainWindow::OnStopRecording);
  connect(m_menu_bar, &MenuBar::ExportRecording, this, &MainWindow::OnExportRecording);
  connect(m_menu_bar, &MenuBar::ShowTASInput, this, &MainWindow::ShowTASInput);

  // View
  connect(m_menu_bar, &MenuBar::ShowList, m_game_list, &GameList::SetListView);
  connect(m_menu_bar, &MenuBar::ShowGrid, m_game_list, &GameList::SetGridView);
  connect(m_menu_bar, &MenuBar::PurgeGameListCache, m_game_list, &GameList::PurgeCache);
  connect(m_menu_bar, &MenuBar::ShowSearch, m_search_bar, &SearchBar::Show);

  connect(m_menu_bar, &MenuBar::ColumnVisibilityToggled, m_game_list,
          &GameList::OnColumnVisibilityToggled);

  connect(m_menu_bar, &MenuBar::GameListPlatformVisibilityToggled, m_game_list,
          &GameList::OnGameListVisibilityChanged);
  connect(m_menu_bar, &MenuBar::GameListRegionVisibilityToggled, m_game_list,
          &GameList::OnGameListVisibilityChanged);

  connect(m_menu_bar, &MenuBar::ShowAboutDialog, this, &MainWindow::ShowAboutDialog);

  connect(m_game_list, &GameList::SelectionChanged, m_menu_bar, &MenuBar::SelectionChanged);
  connect(this, &MainWindow::ReadOnlyModeChanged, m_menu_bar, &MenuBar::ReadOnlyModeChanged);
  connect(this, &MainWindow::RecordingStatusChanged, m_menu_bar, &MenuBar::RecordingStatusChanged);
}

 // preps the installation for KAR Netplay
void MainWindow::PrepInstallForKARNetplay()
{
  // informs the user what we are doing
  if (ModalMessageBox::question(
          this, tr("Prepare for KAR Netplay! Just getting our Waddle Dees in a row."),
          tr("Your KARphin instance will be prepared for KAR Netplay.\n\n"
             "---We will NOT override any of your global Dolphin settings.---\n\nThis instance of KARphin will work by LOCAL settings only. Meaning the ones in the same folder. "
          "KARphin will be closed after it's preped, but don't worry, you can open it up again and it will be all ready to get into Net Play!"),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::NoButton, Qt::ApplicationModal) != QMessageBox::Yes)
    return;

  // force style to dark
  Settings::Instance().SetStyleType(Settings::StyleType::Dark);
  Settings::Instance().ApplyStyle();

  // changes theme color to light pink
  Settings::Instance().SetThemeName(tr("Clean Pink"));

  // search first for a KAR Workshop file to get the ROMs and other tools

  // verify it can find the Hack Pack

  // get the latest Hack Pack gekko codes
  std::string gekkoCodes = """[Gecko]\n"""
"""$━━━━━━━━━━━━ Fullscreen codes ━━━━━━━━━━━━"""
"""*For netplay, it is necessary to have 1 of the fullscreen codes enabled. Having none or more than 1 fullscreen code enabled will cause desyncs\n"""
"""$P1 Fullscreen\n"""
"""C202DF30 00000003\n"""
"""2C1A0000 4082000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""C2020764 00000003\n"""
"""2C1B0000 4082000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""*When enabling this code, your session will desync when City Trial begins. This is NORMAL!\n"""
"""$P2 Fullscreen\n"""
"""C202DF30 00000003\n"""
"""2C1A0001 4082000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""C2020764 00000003\n"""
"""2C1B0001 4082000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""*When enabling this code, your session will desync when City Trial begins. This is NORMAL!\n"""
"""$P3 Fullscreen\n"""
"""C202DF30 00000003\n"""
"""2C1A0002 4082000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""C2020764 00000003\n"""
"""2C1B0002 4082000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""*When enabling this code, your session will desync when City Trial begins. This is NORMAL!\n"""
"""$P4 Fullscreen\n"""
"""C202DF30 00000003\n"""
"""2C1A0003 4082000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""C2020764 00000003\n"""
"""2C1B0003 4082000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""*When enabling this code, your session will desync when City Trial begins. This is NORMAL!\n"""
"""$━━━━━━━━━━━━ Netplay Default Codes ━━━━━━━━━━━━\n"""
"""*These codes should always be enabled during netplay\n"""
"""$New Kirby Lag Reduction [UnclePunch]\n"""
"""044112ec 60000000 #disable original VI\n"""
"""0441140c 48000018 #skip the assert from disabling\n"""
"""C2006B90 00000003 #call new VI\n"""
"""3DC0803D 61CEE164\n"""
"""7DC903A6 4E800421\n"""
"""38600000 00000000\n"""
"""04062848 4BFFFD85 #remove pad renew alarm\n"""
"""04005a94 3C608006 #set pad renew as VI callback\n"""
"""04005a98 606325CC #set pad renew as VI callback\n"""
"""$KAR Netplay Community Settings v3 [UnclePunch,Cheetaboy,MagicScrumpy]\n"""
"""025367BC 003BFFFF\n"""
"""025369FC 003BFFFF\n"""
"""025368d4 003BFFFF\n"""
"""04536EE8 FFFFFFFF\n"""
"""04536EEC 00000000\n"""
"""00535a02 000000FE\n"""
"""04452798 38600000\n"""
"""04047750 38600003\n"""
"""0401F580 39000001\n"""
"""0401F548 39000063\n"""
"""0401F540 2C000001\n"""
"""0401F578 2C000063\n"""
"""040B6980 60000000\n"""
"""C2033B78 0000000C\n"""
"""3DE08053 61EF6EDC\n"""
"""7E1678AE 3DC00010\n"""
"""7F867000 419E0014\n"""
"""3DC00020 7F867000\n"""
"""419E0030 48000030\n"""
"""3A600001 7E7679AE\n"""
"""38800004 38C0FFFE\n"""
"""7EC3B378 7C852378\n"""
"""3E208007 62311D00\n"""
"""7E2903A6 4E800420\n"""
"""7E5679AE 4E800020\n"""
"""60000000 00000000\n"""
"""$2v2 Team Battle [UnclePunch,container12345]\n"""
"""#Machine Hitbox\n"""
"""C21D70E4 00000009\n"""
"""807D0004 2C030000\n"""
"""41820038 809E0004\n"""
"""2C040000 4182002C\n"""
"""8063002C 8084002C\n"""
"""8863000A 8884000A\n"""
"""7C032000 40820014\n"""
"""3D80801D 618C70F0\n"""
"""7D8903A6 4E800420\n"""
"""807D0660 00000000\n"""
"""#Ability Projectile Hitbox\n"""
"""C21D71B0 0000000B\n"""
"""807F002C 80630008\n"""
"""2C030000 41820040\n"""
"""809D0004 2C040000\n"""
"""41820034 8063002C\n"""
"""8084002C 7C032000\n"""
"""41820024 8863000A\n"""
"""8884000A 7C032000\n"""
"""40820014 3D80801D\n"""
"""618C71C4 7D8903A6\n"""
"""4E800420 7FE3FB78\n"""
"""60000000 00000000\n"""
"""#Ability Melee Hitbox\n"""
"""C21D7034 00000008\n"""
"""809E0004 2C040000\n"""
"""4182002C 807F002C\n"""
"""8084002C 8863000A\n"""
"""8884000A 7C032000\n"""
"""40820014 3D80801D\n"""
"""618C7048 7D8903A6\n"""
"""4E800420 7FE3FB78\n"""
"""60000000 00000000\n"""
"""#TimeBomb Detonation\n"""
"""C222933C 00000008\n"""
"""807E0008 7C03F800\n"""
"""4182002C 8063002C\n"""
"""809F002C 8863000A\n"""
"""8884000A 7C032000\n"""
"""40820014 3D808022\n"""
"""618C9378 7D8903A6\n"""
"""4E800420 7FE3FB78\n"""
"""60000000 00000000\n"""
"""#Allow same color(VIEW NOTE)\n"""
"""04021808 4800010C\n"""
"""0402A5AC 48000064\n"""
"""0402F3EC 4800017C\n"""
"""*Players with the same color will be a team, who cannot damage each other.\n"""
"""$City Trial Starts at 6 Mins [UnclePunch]\n"""
"""040075bc 38C00168\n"""
"""040070e8 38C00168\n"""
"""$Remove player indicator and map position during fog event [container12345]\n"""
"""C2115984 00000006\n"""
"""2C030000 41820024\n"""
"""808D0618 2C840000\n"""
"""41860014 8084002C\n"""
"""80840008 2C04000E\n"""
"""48000008 2C030000\n"""
"""60000000 00000000\n"""
"""$Display OSReport Upon Crashing [UnclePunch]\n"""
"""0443ff60 60000000\n"""
"""0443f848 48000268\n"""
"""*Enable this code in order to assist with debugging crashes.\n"""
"""$Invincible Kirby when not on a machine [container12345]\n"""
"""0418FAA4 48000050\n"""
"""$━━━━━━━━━━━━ Main Codes ━━━━━━━━━━━━\n"""
"""*Basic utility codes for Kirby Air Ride.\n"""
"""$Spawned machine won't break until someone ride on it [container12345]\n"""
"""041DF2A0 60000000\n"""
"""045E1C78 7F800000\n"""
"""$Legendary machine parts appear every match [container12345]\n"""
"""040ED09C 60000000\n"""
"""040ED1C0 60000000\n"""
"""$Allow machine select on City Trial [container12345]\n"""
"""0402E5BC 60000000\n"""
"""0402DE84 60000000\n"""
"""040315E0 48000010\n"""
"""0403434C 48000010\n"""
"""04034CA0 48000010\n"""
"""040357A8 48000010\n"""
"""04035E18 48000010\n"""
"""0403612C 48000010\n"""
"""040397EC 48000010\n"""
"""041352E8 480274C1\n"""
"""040389CC 38000001\n"""
"""C20390A4 00000003\n"""
"""9B3F0004 3C008002\n"""
"""6000E3C4 7C0903A6\n"""
"""4E800421 00000000\n"""
"""*Buggy UI but works.\n"""
"""$CPU AI always aggressive [container12345]\n"""
"""04262E04 38600008\n"""
"""*Destruction Derby AI on all gamemodes except Top Ride. CPU won't look for new machines, because Destruction Derby AI normally doesn't need to. Therefore, combine this code with the \"Allow machine select on City Trial\" code to get the CPUs on other machines than Compact Star\n"""
"""$All machines selectable in Stadium mode [container12345]\n"""
"""0402E538 60000000\n"""
"""0402E538 60000000\n"""
"""0402E5A0 60000000\n"""
"""0402E6F0 60000000\n"""
"""$All Craft in Air Ride [Gamemasterplc]\n"""
"""04020BBC 60000000\n"""
"""04020B20 60000000\n"""
"""$Enable Dedede and Meta Knight on City Trial [container12345]\n"""
"""04112CD8 48000074\n"""
"""04117EB0 60000000\n"""
"""04118858 60000000\n"""
"""04119E7C 60000000\n"""
"""0411F03C 60000000\n"""
"""041240AC 60000000\n"""
"""04125F2C 60000000\n"""
"""$All players start with Meta Knight on City Trial (VIEW NOTE) [container12345]\n"""
"""0402DE80 38000013\n"""
"""*Must enable \"Enable Dedede and Meta Knight on City Trial to use this code\n"""
"""$Disable \"Enable getting out of a vehicle\" code for HackPack [container12345]\n"""
"""C21918E0 00000003\n"""
"""3C608000 6063ACB0\n"""
"""7C6903A6 4E800421\n"""
"""7C630774 00000000\n"""
"""$16:9 Aspect Ratio v2 [gamemasterplc]\n"""
"""040647F4 C05E0040\n"""
"""C2064588 00000006\n"""
"""C05C0044 3C808010\n"""
"""80610114 7C041800\n"""
"""386100B4 41800014\n"""
"""3C803FAB 90830000\n"""
"""C0630000 EC4300B2\n"""
"""60000000 00000000\n"""
"""C2401508 00000009\n"""
"""3821FFFC 3C803F40\n"""
"""90810000 809E004C\n"""
"""3CA04280 7C042800\n"""
"""41820020 C0210000\n"""
"""C0010030 EC010032\n"""
"""D0010030 C001003C\n"""
"""EC010032 D001003C\n"""
"""38210004 7FE4FB78\n"""
"""60000000 00000000\n"""
"""0427AE50 4E800020\n"""
"""C20D7F08 00000004\n"""
"""3821FFFC 3C603E40\n"""
"""90610000 C3A10000\n"""
"""C01E0044 EC00E824\n"""
"""38210004 00000000\n"""
"""*Play in 16:9. Note that this code will cause desyncs if other players haven't enabled this code, just like any other gecko codes can desync a lobby when unmatched\n"""
"""$Remove Stats Limit [container12345,gamemasterplc]\n"""
"""04194DEC 4800000C\n"""
"""04194E40 60000000\n"""
"""04194ED4 60000000\n"""
"""04194F28 60000000\n"""
"""04195020 48000060\n"""
"""041CAB14 4800001C\n"""
"""041CAC64 4800001C\n"""
"""$Press Y to Quick Spin [container12345]\n"""
"""C2191A3C 00000005\n"""
"""807E03E4 5463AFFF\n"""
"""41A20018 7C6C42E6\n"""
"""546307BC 3803FFFF\n"""
"""901F0000 38600001\n"""
"""60000000 00000000\n"""
"""$Enable Quick Spin in the air [container12345]\n"""
"""C21AC170 00000005\n"""
"""2C030000 4082001C\n"""
"""3C60801B 60637E80\n"""
"""7C6903A6 7FE3FB78\n"""
"""4E800421 2C030000\n"""
"""60000000 00000000\n"""
"""$Enable machine collision for Air Ride and Stadium [container12345]\n"""
"""041D74E8 60000000\n"""
"""041D762C 60000000\n"""
"""$Disable machine collision for City Trial [container12345]\n"""
"""0400A214 38600000"""
"""$Respawn like Destruction Derby when destroyed [container12345]\n"""
"""041A5178 60000000\n"""
"""$Infinite Jumps\n"""
"""C21BD7C4 00000003\n"""
"""2C030001 4182000C\n"""
"""38030001 48000008\n"""
"""7C601B78 00000000\n"""
"""$Enable Quick Spin with Plasma, Bomb and Mike [container12345]\n"""
"""041B7E9C 60000000\n"""
"""041B1530 4BFF9E08\n"""
"""041B40C8 93E1000C\n"""
"""041B40CC 7C7F1B78\n"""
"""041B40D0 4BFDCB85\n"""
"""041B40D4 4BFF7264\n"""
"""C21AB1C4 00000005\n"""
"""38800021 80A30454\n"""
"""2C050004 40A2000C\n"""
"""3880005A 48000010\n"""
"""2C050007 40A20008\n"""
"""38800062 00000000\n"""
"""$Allow CPUs on City Trial Free Run [container12345]\n"""
"""0403B458 60000000\n"""
"""$Allow CPUs on Air Ride Free Run [container12345]\n"""
"""0402A328 60000000\n"""
"""$Unlock Everything Gold Checklist [Cheetaboy]\n"""
"""025367BC 003BFFFF\n"""
"""025369FC 003BFFFF\n"""
"""025368d4 003BFFFF\n"""
"""04536EE8 FFFFFFFF\n"""
"""04536EEC 00000000\n"""
"""$Colorful menu [container12345]\n"""
"""C23FAC58 00000013\n"""
"""8A860004 8AA60005\n"""
"""8AC60006 7C14B000\n"""
"""40810024 7C16A800\n"""
"""4180000C 3AD60001\n"""
"""48000060 7C14A800\n"""
"""4180000C 3AB5FFFF\n"""
"""48000050 7C16A800\n"""
"""40810024 7C15A000\n"""
"""4180000C 3AB50001\n"""
"""48000038 7C16A000\n"""
"""4180000C 3A94FFFF\n"""
"""48000028 7C15A000\n"""
"""4081002C 7C14B000\n"""
"""4180000C 3A940001\n"""
"""48000010 7C15B000\n"""
"""41800008 3AD6FFFF\n"""
"""9A860004 9AA60005\n"""
"""9AC60006 80060004\n"""
"""60000000 00000000\n"""
"""*A color morphing effect on the menu UI.\n"""
"""$Enable getting out of a vehicle on Air Ride and Stadium (VIEW NOTE) [container12345]\n"""
"""041918EC 38600001\n"""
"""*MUST ENABLE CODE \"Fix crashing when Kirby cross the finish line without machine\" to prevent crashing\n"""
"""$Fix crashing when Kirby cross the finish line without machine [container12345]\n"""
"""C21C7534 00000003\n"""
"""2C030000 4182000C\n"""
"""A0030000 48000008\n"""
"""38000000 00000000\n"""
"""$Unrestricted City Trial Match Time Limit [magicscrumpy + gamemasterplc]\n"""
"""0401F580 39000001\n"""
"""0401F548 39000063\n"""
"""0401F540 2C000001\n"""
"""0401F578 2C000063\n"""
"""$Unrestricted Camera [Cheetaboy]\n"""
"""040B6980 60000000\n"""
"""$Infinite Jumps All Players [Shank + UnclePunch]\n"""
"""C21BD7C4 00000003\n"""
"""2C030001 4182000C\n"""
"""38030001 48000008\n"""
"""7C601B78 00000000\n"""
"""$Disable conveyor [container12345]\n"""
"""040E8058 480002BC\n"""
"""$Disable switch [container12345]\n"""
"""040E86DC 4E800020\n"""
"""$Infinite Acceleration v3 [gamemasterplc, container12345]\n"""
"""041EC0C4 38600000\n"""
"""041F7948 38600000\n"""
"""$Infinite Glide Time\n"""
"""041C612C 60000000\n"""
"""$Disable CPU cheating on VS. King Dedede [container12345]\n"""
"""04040CBC 48000070\n"""
"""04040CBC 48000070\n"""
"""$Disable Moving With DPad [UnclePunch]\n"""
"""0418f0c8 60000000\n"""
"""$Disable L/R Jumping and Charging [UnclePunch]\n"""
"""0418efe4 60000000\n"""
"""$Invulnerable (gamemasterplc)\n"""
"""0418D080 38600001\n"""
"""$No enemies on Air Ride [container12345]\n"""
"""040133F0 38600000\n"""
"""$The race will continue on Top Ride until all CPU reach a goal (VIEW NOTE) [container12345]\n"""
"""042A0164 60000000\n"""
"""042A01F0 60000000\n"""
"""042A027C 60000000\n"""
"""042A0308 60000000\n"""
"""*Enable the code below in order to avoid getting stuck in properties graph.\n"""
"""$Avoid getting stuck in the properties graph when all players are CPUs [container12345]\n"""
"""04045DB4 60000000\n"""
"""04046374 60000000\n"""
"""$One Hit Kill [container12345]\n"""
"""041E1F44 EC210828\n"""
"""$Remove the arrow in front of machines [container12345]\n"""
"""041C70C4 48000014\n"""
"""$Kirby Air Surf v3 [container12345]\n"""
"""C21A63B4 00000008\n"""
"""806D07F8 2C030035\n"""
"""41820030 2C040003\n"""
"""41800028 2C040044\n"""
"""4181000C 38840095\n"""
"""48000018 2C040064\n"""
"""41800010 2C040075\n"""
"""41810008 38840076\n"""
"""7C832378 00000000\n"""
"""C21A63A8 00000009\n"""
"""2C040005 41800014\n"""
"""2C040006 4181000C\n"""
"""388401CF 48000030\n"""
"""2C040044 4181000C\n"""
"""38840095 48000020\n"""
"""2C040064 41800014\n"""
"""2C040069 4181000C\n"""
"""38840076 48000008\n"""
"""3884016A 00000000\n"""
"""$Machine is destroyed when hitting a wall [container12345]\n"""
"""C22312F0 00000007\n"""
"""2C1C0000 41820028\n"""
"""807C002C 806306F8\n"""
"""80630044 80630170\n"""
"""2C030000 41820010\n"""
"""38000000 807C002C\n"""
"""90030A18 7F83E378\n"""
"""60000000 00000000\n"""
"""$Machines get stuck when hitting a wall [container12345]\n"""
"""041905AC 38600001\n"""
"""041D2CC0 38000001\n"""
"""$Machines take damage when hitting a wall [container12345]\n"""
"""C22312F0 00000009\n"""
"""2C1C0000 41820038\n"""
"""807C002C 806306F8\n"""
"""80630044 80630170\n"""
"""2C030000 41820020\n"""
"""3C003F00 807C002C\n"""
"""C2830A18 90030A18\n"""
"""C2A30A18 FE94A828\n"""
"""D2830A18 7F83E378\n"""
"""60000000 00000000\n"""
"""$Random machine in title screen [container12345]\n"""
"""C200D358 00000005\n"""
"""3C608041 6063E668\n"""
"""7C6903A6 38600011\n"""
"""4E800421 7C741B78\n"""
"""38600000 7E84A378\n"""
"""60000000 00000000\n"""
"""0400D398 7E84A378\n"""
"""$Flashy menu [container12345]\n"""
"""C23FAC58 00000002\n"""
"""80060004 5400083E\n"""
"""90060004 00000000\n"""
"""$Allow same color [container12345]\n"""
"""04021808 4800010C\n"""
"""0402A5AC 48000064\n"""
"""0402F3EC 4800017C\n"""
"""$2v2 Team Battle (Collision only) (Read Notes) [container12345]\n"""
"""C218D7D0 0000000D\n"""
"""2C030000 41820060\n"""
"""38600002 7C6903A6\n"""
"""3C608056 80A1004C\n"""
"""80C10030 8003AA30\n"""
"""8083B33C 7C050000\n"""
"""40820010 7C062000\n"""
"""40820008 48000028\n"""
"""7C052000 40820010\n"""
"""7C060000 40820008\n"""
"""48000014 38631218\n"""
"""4200FFCC 38600001\n"""
"""48000008 38600000\n"""
"""2C030000 00000000\n"""
"""*This is an early version of the 2v2 team code. It is based on port and only affects machine collision. P1 & P2 are against P3 and P4.\n"""
"""$Disable Course Animations [gamemasterplc]\n"""
"""040DB8D8 4E800020\n"""
"""$Disable background [container12345]\n"""
"""040D8238 4800006C\n"""
"""$Pause during game start [container12345]\n"""
"""04041050 4800000C\n"""
"""$Reduce CPU Quick Spin [container12345]\n"""
"""042750F4 60000000\n"""
"""$Machines always successfully land [container12345]\n"""
"""041cdb08 60000000\n"""
"""041cf588 38600001\n"""
"""$No 1-Kit KOs [container12345]\n"""
"""C21E1F44 00000004\n"""
"""C042B608 FC011040\n"""
"""FC21F828 40810010\n"""
"""FC011040 41810008\n"""
"""FC201090 00000000\n"""
"""*Originally titled: 1HP handicap like RtDL\n"""
"""$Disable machines sticking to a wall [container12345]\n"""
"""041D2B4C 480001B8\n"""
"""$Disable machines sticking to a floor [container12345]\n"""
"""041D2D58 4800001C\n"""
"""$Skip Memory Card Nag and Tutorial Videos [UnclePunch]\n"""
"""00535a02 000000FE\n"""
"""04452798 38600000\n"""
"""$Boot to Main Menu [UnclePunch]\n"""
"""04047750 38600003\n"""
"""$Skip Intro Cutscene [gamemasterplc]\n"""
"""0400D3A8 480000E0\n"""
"""$Kirby can't get up when his machine is destroyed [container12345]\n"""
"""041A5648 60000000\n"""
"""041BD8D8 38600000\n"""
"""041BD8DC 4E800020\n"""
"""$Disable camera tilting when turning [container12345]\n"""
"""040B390C 60000000\n"""
"""$X and Y Cycles Ability Costumes [UnclePunch]\n"""
"""C218EF24 00000013\n"""
"""887F0457 2C0300FF\n"""
"""40820084 887F02B7\n"""
"""2C03001C 40820078\n"""
"""807F03E4 2C030400\n"""
"""41820010 2C030800\n"""
"""41820020 48000060\n"""
"""807F000C 38630001\n"""
"""2C03000A 40810008\n"""
"""38600000 48000018\n"""
"""807F000C 3863FFFF\n"""
"""2C030000 40800008\n"""
"""3860000A 907F000C\n"""
"""3C60801A 60637D70\n"""
"""7C6903A6 7FE3FB78\n"""
"""4E800421 3C60801A\n"""
"""60636640 7C6903A6\n"""
"""7FE3FB78 809F000C\n"""
"""4E800421 7FE3FB78\n"""
"""60000000 00000000\n"""
"""$Increase speed cap (x2) [container12345]\n"""
"""C21EC0C0 00000004\n"""
"""C024001C 3C004000\n"""
"""9004001C C004001C\n"""
"""D024001C EC210032\n"""
"""60000000 00000000\n"""
"""C21F7944 00000004\n"""
"""C0240060 3C004000\n"""
"""90040060 C0040060\n"""
"""D0240060 EC210032\n"""
"""60000000 00000000\n"""
"""$━━━━━━━━━━━━ Local Netplay ━━━━━━━━━━━━"""
"""*To make sure your local friends can join and play on the same pc during netplay lobbies, use 1 of them instead of a normal fullscreen code and it will work without desyncing\n"""
"""$P1 and P2 screen\n"""
"""C202DF30 00000003\n"""
"""2C1A0002 4080000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""C2020764 00000003\n"""
"""2C1B0002 4080000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""$P3 and P4 screen\n"""
"""C202DF30 00000003\n"""
"""2C1A0001 4081000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""C2020764 00000003\n"""
"""2C1B0001 4081000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""$P1, P2 and P3 screen\n"""
"""C202DF30 00000003\n"""
"""2C1A0003 4182000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""C2020764 00000003\n"""
"""2C1B0003 4182000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""$P2, P3 and P4 screen\n"""
"""C202DF30 00000003\n"""
"""2C1A0000 4182000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""C2020764 00000003\n"""
"""2C1B0000 4182000C\n"""
"""38600001 48000008\n"""
"""38600000 00000000\n"""
"""$-------=======<<< CREDITS >>>=======-------\n"""
"""$<<< Hack Pack >>> [UnclePunch]\n"""
"""*Hack pack creator, ASM god!\n"""
"""$<<< Gecko Codes >>> [container12345]\n"""
"""*200+ Gecko Codes, Kirby Air Ride's biggest fan\n"""
"""$<<< Website >>> [Eternalll]\n"""
"""*Website hosting/development, netplay builds, gecko code compiler\n"""
"""$<<< Alt Vehicle Packs >>> [A_Y, perry1853108]\n"""
"""*Alternative machine packs for the hack pack\n"""
"""$<<< Other Codes >>> [Cheetaboy, gamemasterplc]\n"""
"""*More gecko codes, guides and walkthroughs\n"""
"""$<<< Other Mentions >>> [Kirby Workshop Discord]\n"""
"""*Support, contribution, ideas, our community\n"""
"""[Gecko_Enabled]\n"""
"""$P1 Fullscreen\n"""
"""$New Kirby Lag Reduction\n"""
"""$KAR Netplay Community Settings v3\n"""
"""$2v2 Team Battle\n"""
"""$City Trial Starts at 6 Mins\n"""
"""$Remove player indicator and map position during fog event\n"""
"""$Display OSReport Upon Crashing\n"""
"""$Invincible Kirby when not on a machine\n";

  //write base dolphin settings files
  QFile file(tr("User/Config/Dolphin.ini"));
  file.open(QIODevice::WriteOnly);
  file.write("[Analytics]\nID = 8becf168e11a9b2bf56d903e78eefb03\nPermissionAsked = "
             "True\n[Core]\nAutoDiscChange = False\nCPUThread = True\nEnableCheats = "
             "True\nOverrideRegionSettings = False\n[NetPlay]\nTraversalChoice = traversal\n[General]\nISOPath0 = C:\\Users\\Jas\\Desktop\\ROMs\\GC\\Kirby Air Ride Hack Pack v1.01\nISOPaths = 1\n");

  //writes gekko codes
  file.close();
  file.setFileName(tr("User/GameSettings/KHPE01.ini"));
  file.open(QIODevice::WriteOnly);
  file.write(gekkoCodes.c_str());

  //closes KARphin
  close();
}

// syncs this instance of KARphin with KAR Workshop
void MainWindow::SyncKARphinInstanceWithKARWorkshop()
{
  //search for KAR Workshop
}

void MainWindow::ConnectHotkeys()
{
  connect(m_hotkey_scheduler, &HotkeyScheduler::Open, this, &MainWindow::Open);
  connect(m_hotkey_scheduler, &HotkeyScheduler::ChangeDisc, this, &MainWindow::ChangeDisc);
  connect(m_hotkey_scheduler, &HotkeyScheduler::EjectDisc, this, &MainWindow::EjectDisc);
  connect(m_hotkey_scheduler, &HotkeyScheduler::ExitHotkey, this, &MainWindow::close);
  connect(m_hotkey_scheduler, &HotkeyScheduler::UnlockCursor, this, &MainWindow::UnlockCursor);
  connect(m_hotkey_scheduler, &HotkeyScheduler::TogglePauseHotkey, this, &MainWindow::TogglePause);
  connect(m_hotkey_scheduler, &HotkeyScheduler::ActivateChat, this, &MainWindow::OnActivateChat);
  connect(m_hotkey_scheduler, &HotkeyScheduler::RequestGolfControl, this,
          &MainWindow::OnRequestGolfControl);
  connect(m_hotkey_scheduler, &HotkeyScheduler::RefreshGameListHotkey, this,
          &MainWindow::RefreshGameList);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StopHotkey, this, &MainWindow::RequestStop);
  connect(m_hotkey_scheduler, &HotkeyScheduler::ResetHotkey, this, &MainWindow::Reset);
  connect(m_hotkey_scheduler, &HotkeyScheduler::ScreenShotHotkey, this, &MainWindow::ScreenShot);
  connect(m_hotkey_scheduler, &HotkeyScheduler::FullScreenHotkey, this, &MainWindow::FullScreen);

  connect(m_hotkey_scheduler, &HotkeyScheduler::StateLoadSlot, this, &MainWindow::StateLoadSlotAt);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StateSaveSlot, this, &MainWindow::StateSaveSlotAt);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StateLoadLastSaved, this,
          &MainWindow::StateLoadLastSavedAt);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StateLoadUndo, this, &MainWindow::StateLoadUndo);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StateSaveUndo, this, &MainWindow::StateSaveUndo);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StateSaveOldest, this,
          &MainWindow::StateSaveOldest);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StateSaveFile, this, &MainWindow::StateSave);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StateLoadFile, this, &MainWindow::StateLoad);

  connect(m_hotkey_scheduler, &HotkeyScheduler::StateLoadSlotHotkey, this,
          &MainWindow::StateLoadSlot);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StateSaveSlotHotkey, this,
          &MainWindow::StateSaveSlot);
  connect(m_hotkey_scheduler, &HotkeyScheduler::SetStateSlotHotkey, this,
          &MainWindow::SetStateSlot);
  connect(m_hotkey_scheduler, &HotkeyScheduler::IncrementSelectedStateSlotHotkey, this,
          &MainWindow::IncrementSelectedStateSlot);
  connect(m_hotkey_scheduler, &HotkeyScheduler::DecrementSelectedStateSlotHotkey, this,
          &MainWindow::DecrementSelectedStateSlot);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StartRecording, this,
          &MainWindow::OnStartRecording);
  connect(m_hotkey_scheduler, &HotkeyScheduler::PlayRecording, this, &MainWindow::OnPlayRecording);
  connect(m_hotkey_scheduler, &HotkeyScheduler::ExportRecording, this,
          &MainWindow::OnExportRecording);
  connect(m_hotkey_scheduler, &HotkeyScheduler::ConnectWiiRemote, this,
          &MainWindow::OnConnectWiiRemote);
  connect(m_hotkey_scheduler, &HotkeyScheduler::ToggleReadOnlyMode, [this] {
    auto& movie = Core::System::GetInstance().GetMovie();
    bool read_only = !movie.IsReadOnly();
    movie.SetReadOnly(read_only);
    emit ReadOnlyModeChanged(read_only);
  });

  connect(m_hotkey_scheduler, &HotkeyScheduler::Step, m_code_widget, &CodeWidget::Step);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StepOver, m_code_widget, &CodeWidget::StepOver);
  connect(m_hotkey_scheduler, &HotkeyScheduler::StepOut, m_code_widget, &CodeWidget::StepOut);
  connect(m_hotkey_scheduler, &HotkeyScheduler::Skip, m_code_widget, &CodeWidget::Skip);

  connect(m_hotkey_scheduler, &HotkeyScheduler::ShowPC, m_code_widget, &CodeWidget::ShowPC);
  connect(m_hotkey_scheduler, &HotkeyScheduler::SetPC, m_code_widget, &CodeWidget::SetPC);

  connect(m_hotkey_scheduler, &HotkeyScheduler::ToggleBreakpoint, m_code_widget,
          &CodeWidget::ToggleBreakpoint);
  connect(m_hotkey_scheduler, &HotkeyScheduler::AddBreakpoint, m_code_widget,
          &CodeWidget::AddBreakpoint);

  connect(m_hotkey_scheduler, &HotkeyScheduler::SkylandersPortalHotkey, this,
          &MainWindow::ShowSkylanderPortal);
  connect(m_hotkey_scheduler, &HotkeyScheduler::InfinityBaseHotkey, this,
          &MainWindow::ShowInfinityBase);
}

void MainWindow::ConnectToolBar()
{
  addToolBar(m_tool_bar);

  connect(m_tool_bar, &ToolBar::OpenPressed, this, &MainWindow::Open);
  connect(m_tool_bar, &ToolBar::RefreshPressed, this, &MainWindow::RefreshGameList);

  connect(m_tool_bar, &ToolBar::PlayPressed, this, [this]() { Play(); });
  connect(m_tool_bar, &ToolBar::PausePressed, this, &MainWindow::Pause);
  connect(m_tool_bar, &ToolBar::StopPressed, this, &MainWindow::RequestStop);
  connect(m_tool_bar, &ToolBar::FullScreenPressed, this, &MainWindow::FullScreen);
  connect(m_tool_bar, &ToolBar::ScreenShotPressed, this, &MainWindow::ScreenShot);
  connect(m_tool_bar, &ToolBar::SettingsPressed, this, &MainWindow::ShowSettingsWindow);
  connect(m_tool_bar, &ToolBar::ControllersPressed, this, &MainWindow::ShowControllersWindow);
  connect(m_tool_bar, &ToolBar::GraphicsPressed, this, &MainWindow::ShowGraphicsWindow);

  connect(m_tool_bar, &ToolBar::StepPressed, m_code_widget, &CodeWidget::Step);
  connect(m_tool_bar, &ToolBar::StepOverPressed, m_code_widget, &CodeWidget::StepOver);
  connect(m_tool_bar, &ToolBar::StepOutPressed, m_code_widget, &CodeWidget::StepOut);
  connect(m_tool_bar, &ToolBar::SkipPressed, m_code_widget, &CodeWidget::Skip);
  connect(m_tool_bar, &ToolBar::ShowPCPressed, m_code_widget, &CodeWidget::ShowPC);
  connect(m_tool_bar, &ToolBar::SetPCPressed, m_code_widget, &CodeWidget::SetPC);
}

void MainWindow::ConnectGameList()
{
  connect(m_game_list, &GameList::GameSelected, this, [this]() { Play(); });
  connect(m_game_list, &GameList::NetPlayHost, this, &MainWindow::NetPlayHost);
  connect(m_game_list, &GameList::OnStartWithRiivolution, this,
          &MainWindow::ShowRiivolutionBootWidget);

  connect(m_game_list, &GameList::OpenGeneralSettings, this, &MainWindow::ShowGeneralWindow);
  connect(m_game_list, &GameList::OpenGraphicsSettings, this, &MainWindow::ShowGraphicsWindow);
}

void MainWindow::ConnectRenderWidget()
{
  m_rendering_to_main = false;
  m_render_widget->hide();
  connect(m_render_widget, &RenderWidget::Closed, this, &MainWindow::ForceStop);
  connect(m_render_widget, &RenderWidget::FocusChanged, this, [this](bool focus) {
    if (m_render_widget->isFullScreen())
      SetFullScreenResolution(focus);
  });
}

void MainWindow::ConnectHost()
{
  connect(Host::GetInstance(), &Host::RequestStop, this, &MainWindow::RequestStop);
}

void MainWindow::ConnectStack()
{
  auto* widget = new QWidget;
  auto* layout = new QVBoxLayout;
  widget->setLayout(layout);

  layout->addWidget(m_game_list);
  layout->addWidget(m_search_bar);
  layout->setContentsMargins(0, 0, 0, 0);

  connect(m_search_bar, &SearchBar::Search, m_game_list, &GameList::SetSearchTerm);

  m_stack->addWidget(widget);

  setCentralWidget(m_stack);

  setDockOptions(DockOption::AllowNestedDocks | DockOption::AllowTabbedDocks);
  setTabPosition(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea, QTabWidget::North);
  addDockWidget(Qt::LeftDockWidgetArea, m_log_widget);
  addDockWidget(Qt::LeftDockWidgetArea, m_log_config_widget);
  addDockWidget(Qt::LeftDockWidgetArea, m_code_widget);
  addDockWidget(Qt::LeftDockWidgetArea, m_register_widget);
  addDockWidget(Qt::LeftDockWidgetArea, m_thread_widget);
  addDockWidget(Qt::LeftDockWidgetArea, m_watch_widget);
  addDockWidget(Qt::LeftDockWidgetArea, m_breakpoint_widget);
  addDockWidget(Qt::LeftDockWidgetArea, m_memory_widget);
  addDockWidget(Qt::LeftDockWidgetArea, m_network_widget);
  addDockWidget(Qt::LeftDockWidgetArea, m_jit_widget);
  addDockWidget(Qt::LeftDockWidgetArea, m_assembler_widget);

  tabifyDockWidget(m_log_widget, m_log_config_widget);
  tabifyDockWidget(m_log_widget, m_code_widget);
  tabifyDockWidget(m_log_widget, m_register_widget);
  tabifyDockWidget(m_log_widget, m_thread_widget);
  tabifyDockWidget(m_log_widget, m_watch_widget);
  tabifyDockWidget(m_log_widget, m_breakpoint_widget);
  tabifyDockWidget(m_log_widget, m_memory_widget);
  tabifyDockWidget(m_log_widget, m_network_widget);
  tabifyDockWidget(m_log_widget, m_jit_widget);
  tabifyDockWidget(m_log_widget, m_assembler_widget);
}

void MainWindow::RefreshGameList()
{
  Settings::Instance().ReloadTitleDB();
  Settings::Instance().RefreshGameList();
}

QStringList MainWindow::PromptFileNames()
{
  auto& settings = Settings::Instance().GetQSettings();
  QStringList paths = DolphinFileDialog::getOpenFileNames(
      this, tr("Select a File"),
      settings.value(QStringLiteral("mainwindow/lastdir"), QString{}).toString(),
      QStringLiteral("%1 (*.elf *.dol *.gcm *.iso *.tgc *.wbfs *.ciso *.gcz *.wia *.rvz "
                     "hif_000000.nfs *.wad *.dff *.m3u *.json);;%2 (*)")
          .arg(tr("All GC/Wii files"))
          .arg(tr("All Files")));

  if (!paths.isEmpty())
  {
    settings.setValue(QStringLiteral("mainwindow/lastdir"),
                      QFileInfo(paths.front()).absoluteDir().absolutePath());
  }

  return paths;
}

void MainWindow::ChangeDisc()
{
  std::vector<std::string> paths = StringListToStdVector(PromptFileNames());

  if (paths.empty())
    return;

  auto& system = Core::System::GetInstance();
  system.GetDVDInterface().ChangeDisc(Core::CPUThreadGuard{system}, paths);
}

void MainWindow::EjectDisc()
{
  auto& system = Core::System::GetInstance();
  system.GetDVDInterface().EjectDisc(Core::CPUThreadGuard{system}, DVD::EjectCause::User);
}

void MainWindow::OpenUserFolder()
{
  std::string path = File::GetUserPath(D_USER_IDX);

  QUrl url = QUrl::fromLocalFile(QString::fromStdString(path));
  QDesktopServices::openUrl(url);
}

void MainWindow::Open()
{
  QStringList files = PromptFileNames();
  if (!files.isEmpty())
    StartGame(StringListToStdVector(files));
}

void MainWindow::Play(const std::optional<std::string>& savestate_path)
{
  // If we're in a paused game, start it up again.
  // Otherwise, play the selected game, if there is one.
  // Otherwise, play the default game.
  // Otherwise, play the last played game, if there is one.
  // Otherwise, prompt for a new game.
  if (Core::GetState() == Core::State::Paused)
  {
    Core::SetState(Core::State::Running);
  }
  else
  {
    std::shared_ptr<const UICommon::GameFile> selection = m_game_list->GetSelectedGame();
    if (selection)
    {
      StartGame(selection->GetFilePath(), ScanForSecondDisc::Yes,
                std::make_unique<BootSessionData>(savestate_path, DeleteSavestateAfterBoot::No));
    }
    else
    {
      const QString default_path = QString::fromStdString(Config::Get(Config::MAIN_DEFAULT_ISO));
      if (!default_path.isEmpty() && QFile::exists(default_path))
      {
        StartGame(default_path, ScanForSecondDisc::Yes,
                  std::make_unique<BootSessionData>(savestate_path, DeleteSavestateAfterBoot::No));
      }
      else
      {
        Open();
      }
    }
  }
}

void MainWindow::Pause()
{
  Core::SetState(Core::State::Paused);
}

void MainWindow::TogglePause()
{
  if (Core::GetState() == Core::State::Paused)
  {
    Play();
  }
  else
  {
    Pause();
  }
}

void MainWindow::OnStopComplete()
{
  m_stop_requested = false;
  HideRenderWidget(!m_exit_requested, m_exit_requested);
#ifdef USE_DISCORD_PRESENCE
  if (!m_netplay_dialog->isVisible())
    Discord::UpdateDiscordPresence();
#endif

  SetFullScreenResolution(false);

  if (m_exit_requested || Settings::Instance().IsBatchModeEnabled())
  {
    if (m_assembler_widget->ApplicationCloseRequest())
    {
      QGuiApplication::exit(0);
    }
    else
    {
      m_exit_requested = false;
    }
  }

  // If the current emulation prevented the booting of another, do that now
  if (m_pending_boot != nullptr)
  {
    StartGame(std::move(m_pending_boot));
    m_pending_boot.reset();
  }
}

bool MainWindow::RequestStop()
{
  if (!Core::IsRunning())
  {
    Core::QueueHostJob([this](Core::System&) { OnStopComplete(); }, true);
    return true;
  }

  const bool rendered_widget_was_active =
      m_render_widget->isActiveWindow() && !m_render_widget->isFullScreen();
  QWidget* confirm_parent = (!m_rendering_to_main && rendered_widget_was_active) ?
                                m_render_widget :
                                static_cast<QWidget*>(this);
  const bool was_cursor_locked = m_render_widget->IsCursorLocked();

  if (!m_render_widget->isFullScreen())
    m_render_widget_geometry = m_render_widget->saveGeometry();
  else
    FullScreen();

  if (Config::Get(Config::MAIN_CONFIRM_ON_STOP))
  {
    if (std::exchange(m_stop_confirm_showing, true))
      return true;

    Common::ScopeGuard confirm_lock([this] { m_stop_confirm_showing = false; });

    const Core::State state = Core::GetState();

    // Only pause the game, if NetPlay is not running
    bool pause = !Settings::Instance().GetNetPlayClient();

    if (pause)
      Core::SetState(Core::State::Paused);

    if (rendered_widget_was_active)
    {
      // We have to do this before creating the message box, otherwise we might receive the window
      // activation event before we know we need to lock the cursor again.
      m_render_widget->SetCursorLockedOnNextActivation(was_cursor_locked);
    }

    // This is to avoid any "race conditions" between the "Window Activate" message and the
    // message box returning, which could break cursor locking depending on the order
    m_render_widget->SetWaitingForMessageBox(true);
    auto confirm = ModalMessageBox::question(
        confirm_parent, tr("Confirm"),
        m_stop_requested ? tr("A shutdown is already in progress. Unsaved data "
                              "may be lost if you stop the current emulation "
                              "before it completes. Force stop?") :
                           tr("Do you want to stop the current emulation?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::NoButton, Qt::ApplicationModal);

    // If a user confirmed stopping the emulation, we do not capture the cursor again,
    // even if the render widget will stay alive for a while.
    // If a used rejected stopping the emulation, we instead capture the cursor again,
    // and let them continue playing as if nothing had happened
    // (assuming cursor locking is on).
    if (confirm != QMessageBox::Yes)
    {
      m_render_widget->SetWaitingForMessageBox(false);

      if (pause)
        Core::SetState(state);

      return false;
    }
    else
    {
      m_render_widget->SetCursorLockedOnNextActivation(false);
      // This needs to be after SetCursorLockedOnNextActivation(false) as it depends on it
      m_render_widget->SetWaitingForMessageBox(false);
    }
  }

  OnStopRecording();
  // TODO: Add Debugger shutdown

  if (!m_stop_requested && UICommon::TriggerSTMPowerEvent())
  {
    m_stop_requested = true;

    // Unpause because gracefully shutting down needs the game to actually request a shutdown.
    // TODO: Do not unpause in debug mode to allow debugging until the complete shutdown.
    if (Core::GetState() == Core::State::Paused)
      Core::SetState(Core::State::Running);

    // Tell NetPlay about the power event
    if (NetPlay::IsNetPlayRunning())
      NetPlay::SendPowerButtonEvent();

    return true;
  }

  ForceStop();
#ifdef Q_OS_WIN
  // Allow windows to idle or turn off display again
  SetThreadExecutionState(ES_CONTINUOUS);
#endif
  return true;
}

void MainWindow::ForceStop()
{
  Core::Stop(Core::System::GetInstance());
}

void MainWindow::Reset()
{
  auto& system = Core::System::GetInstance();
  auto& movie = system.GetMovie();
  if (movie.IsRecordingInput())
    movie.SetReset(true);
  system.GetProcessorInterface().ResetButton_Tap();
}

void MainWindow::FrameAdvance()
{
  Core::DoFrameStep();
}

void MainWindow::FullScreen()
{
  // If the render widget is fullscreen we want to reset it to whatever is in
  // settings. If it's set to be fullscreen then it just remakes the window,
  // which probably isn't ideal.
  bool was_fullscreen = m_render_widget->isFullScreen();

  if (!was_fullscreen)
    m_render_widget_geometry = m_render_widget->saveGeometry();

  HideRenderWidget(false);
  SetFullScreenResolution(!was_fullscreen);

  if (was_fullscreen)
  {
    ShowRenderWidget();
  }
  else
  {
    m_render_widget->showFullScreen();
  }
}

void MainWindow::UnlockCursor()
{
  if (!m_render_widget->isFullScreen())
    m_render_widget->SetCursorLocked(false);
}

void MainWindow::ScreenShot()
{
  Core::SaveScreenShot();
}

void MainWindow::ScanForSecondDiscAndStartGame(const UICommon::GameFile& game,
                                               std::unique_ptr<BootSessionData> boot_session_data)
{
  auto second_game = m_game_list->FindSecondDisc(game);

  std::vector<std::string> paths = {game.GetFilePath()};
  if (second_game != nullptr)
    paths.push_back(second_game->GetFilePath());

  StartGame(paths, std::move(boot_session_data));
}

void MainWindow::StartGame(const QString& path, ScanForSecondDisc scan,
                           std::unique_ptr<BootSessionData> boot_session_data)
{
  StartGame(path.toStdString(), scan, std::move(boot_session_data));
}

void MainWindow::StartGame(const std::string& path, ScanForSecondDisc scan,
                           std::unique_ptr<BootSessionData> boot_session_data)
{
  if (scan == ScanForSecondDisc::Yes)
  {
    std::shared_ptr<const UICommon::GameFile> game = m_game_list->FindGame(path);
    if (game != nullptr)
    {
      ScanForSecondDiscAndStartGame(*game, std::move(boot_session_data));
      return;
    }
  }

  StartGame(BootParameters::GenerateFromFile(
      path, boot_session_data ? std::move(*boot_session_data) : BootSessionData()));
}

void MainWindow::StartGame(const std::vector<std::string>& paths,
                           std::unique_ptr<BootSessionData> boot_session_data)
{
  StartGame(BootParameters::GenerateFromFile(
      paths, boot_session_data ? std::move(*boot_session_data) : BootSessionData()));
}

void MainWindow::StartGame(std::unique_ptr<BootParameters>&& parameters)
{
  if (parameters && std::holds_alternative<BootParameters::Disc>(parameters->parameters))
  {
    if (std::get<BootParameters::Disc>(parameters->parameters).volume->IsNKit())
    {
      if (!NKitWarningDialog::ShowUnlessDisabled())
        return;
    }
  }

  // If we're running, only start a new game once we've stopped the last.
  if (Core::GetState() != Core::State::Uninitialized)
  {
    if (!RequestStop())
      return;

    // As long as the shutdown isn't complete, we can't boot, so let's boot later
    m_pending_boot = std::move(parameters);
    return;
  }

  // We need the render widget before booting.
  ShowRenderWidget();

  // Boot up, show an error if it fails to load the game.
  if (!BootManager::BootCore(Core::System::GetInstance(), std::move(parameters),
                             ::GetWindowSystemInfo(m_render_widget->windowHandle())))
  {
    ModalMessageBox::critical(this, tr("Error"), tr("Failed to init core"), QMessageBox::Ok);
    HideRenderWidget();
    return;
  }

#ifdef USE_DISCORD_PRESENCE
  if (!NetPlay::IsNetPlayRunning())
    Discord::UpdateDiscordPresence();
#endif

  if (Config::Get(Config::MAIN_FULLSCREEN))
    m_fullscreen_requested = true;
}

void MainWindow::SetFullScreenResolution(bool fullscreen)
{
  if (Config::Get(Config::MAIN_FULLSCREEN_DISPLAY_RES) == "Auto")
    return;
#ifdef _WIN32

  if (!fullscreen)
  {
    ChangeDisplaySettings(nullptr, CDS_FULLSCREEN);
    return;
  }

  DEVMODE screen_settings;
  memset(&screen_settings, 0, sizeof(screen_settings));
  screen_settings.dmSize = sizeof(screen_settings);
  sscanf(Config::Get(Config::MAIN_FULLSCREEN_DISPLAY_RES).c_str(), "%dx%d",
         &screen_settings.dmPelsWidth, &screen_settings.dmPelsHeight);
  screen_settings.dmBitsPerPel = 32;
  screen_settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

  // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
  ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN);
#elif defined(HAVE_XRANDR) && HAVE_XRANDR
  if (m_xrr_config)
    m_xrr_config->ToggleDisplayMode(fullscreen);
#endif
}

void MainWindow::ShowRenderWidget()
{
  SetFullScreenResolution(false);
  Host::GetInstance()->SetRenderFullscreen(false);

  if (Config::Get(Config::MAIN_RENDER_TO_MAIN))
  {
    // If we're rendering to main, add it to the stack and update our title when necessary.
    m_rendering_to_main = true;

    m_stack->setCurrentIndex(m_stack->addWidget(m_render_widget));
    connect(Host::GetInstance(), &Host::RequestTitle, this, &MainWindow::setWindowTitle);
    m_stack->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_stack->repaint();

    Host::GetInstance()->SetRenderFocus(isActiveWindow());
  }
  else
  {
    // Otherwise, just show it.
    m_rendering_to_main = false;

    m_render_widget->showNormal();
    m_render_widget->restoreGeometry(m_render_widget_geometry);
  }
}

void MainWindow::HideRenderWidget(bool reinit, bool is_exit)
{
  if (m_rendering_to_main)
  {
    // Remove the widget from the stack and reparent it to nullptr, so that it can draw
    // itself in a new window if it wants. Disconnect the title updates.
    m_stack->removeWidget(m_render_widget);
    m_render_widget->setParent(nullptr);
    m_rendering_to_main = false;
    m_stack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    disconnect(Host::GetInstance(), &Host::RequestTitle, this, &MainWindow::setWindowTitle);
    setWindowTitle(QString::fromStdString(Common::GetScmRevStr()));
  }

  // The following code works around a driver bug that would lead to Dolphin crashing when changing
  // graphics backends (e.g. OpenGL to Vulkan). To avoid this the render widget is (safely)
  // recreated
  if (reinit)
  {
    m_render_widget->hide();
    disconnect(m_render_widget, &RenderWidget::Closed, this, &MainWindow::ForceStop);

    m_render_widget->removeEventFilter(this);
    m_render_widget->deleteLater();

    m_render_widget = new RenderWidget;

    m_render_widget->installEventFilter(this);
    connect(m_render_widget, &RenderWidget::Closed, this, &MainWindow::ForceStop);
    connect(m_render_widget, &RenderWidget::FocusChanged, this, [this](bool focus) {
      if (m_render_widget->isFullScreen())
        SetFullScreenResolution(focus);
    });

    // The controller interface will still be registered to the old render widget, if the core
    // has booted. Therefore, we should re-bind it to the main window for now. When the core
    // is next started, it will be swapped back to the new render widget.
    g_controller_interface.ChangeWindow(::GetWindowSystemInfo(windowHandle()).render_window,
                                        is_exit ? ControllerInterface::WindowChangeReason::Exit :
                                                  ControllerInterface::WindowChangeReason::Other);
  }
}

void MainWindow::ShowControllersWindow()
{
  if (!m_controllers_window)
  {
    m_controllers_window = new ControllersWindow(this);
    InstallHotkeyFilter(m_controllers_window);
  }

  SetQWidgetWindowDecorations(m_controllers_window);
  m_controllers_window->show();
  m_controllers_window->raise();
  m_controllers_window->activateWindow();
}

void MainWindow::ShowFreeLookWindow()
{
  if (!m_freelook_window)
  {
    m_freelook_window = new FreeLookWindow(this);
    InstallHotkeyFilter(m_freelook_window);

//#ifdef USE_RETRO_ACHIEVEMENTS
//    connect(m_freelook_window, &FreeLookWindow::OpenAchievementSettings, this,
//            &MainWindow::ShowAchievementSettings);
//#endif  // USE_RETRO_ACHIEVEMENTS
  }

  SetQWidgetWindowDecorations(m_freelook_window);
  m_freelook_window->show();
  m_freelook_window->raise();
  m_freelook_window->activateWindow();
}

void MainWindow::ShowSettingsWindow()
{
  if (!m_settings_window)
  {
    m_settings_window = new SettingsWindow(this);
    InstallHotkeyFilter(m_settings_window);
  }

  SetQWidgetWindowDecorations(m_settings_window);
  m_settings_window->show();
  m_settings_window->raise();
  m_settings_window->activateWindow();
}

void MainWindow::ShowAudioWindow()
{
  ShowSettingsWindow();
  m_settings_window->SelectAudioPane();
}

void MainWindow::ShowGeneralWindow()
{
  ShowSettingsWindow();
  m_settings_window->SelectGeneralPane();
}

void MainWindow::ShowAboutDialog()
{
  AboutDialog about{this};
  SetQWidgetWindowDecorations(&about);
  about.exec();
}

void MainWindow::ShowHotkeyDialog()
{
  if (!m_hotkey_window)
  {
    m_hotkey_window = new MappingWindow(this, MappingWindow::Type::MAPPING_HOTKEYS, 0);
    InstallHotkeyFilter(m_hotkey_window);
  }

  SetQWidgetWindowDecorations(m_hotkey_window);
  m_hotkey_window->show();
  m_hotkey_window->raise();
  m_hotkey_window->activateWindow();
}

void MainWindow::ShowGraphicsWindow()
{
  if (!m_graphics_window)
  {
#ifdef HAVE_XRANDR
    if (GetWindowSystemType() == WindowSystemType::X11)
    {
      m_xrr_config = std::make_unique<X11Utils::XRRConfiguration>(
          static_cast<Display*>(QGuiApplication::platformNativeInterface()->nativeResourceForWindow(
              "display", windowHandle())),
          winId());
    }
#endif
    m_graphics_window = new GraphicsWindow(this);
    InstallHotkeyFilter(m_graphics_window);
  }

  SetQWidgetWindowDecorations(m_graphics_window);
  m_graphics_window->show();
  m_graphics_window->raise();
  m_graphics_window->activateWindow();
}

void MainWindow::ShowNetPlaySetupDialog()
{
  SetQWidgetWindowDecorations(m_netplay_setup_dialog);
  m_netplay_setup_dialog->show();
  m_netplay_setup_dialog->raise();
  m_netplay_setup_dialog->activateWindow();
}

void MainWindow::ShowNetPlayBrowser()
{
  auto* browser = new NetPlayBrowser(this);
  browser->setAttribute(Qt::WA_DeleteOnClose, true);
  connect(browser, &NetPlayBrowser::Join, this, &MainWindow::NetPlayJoin);
  SetQWidgetWindowDecorations(browser);
  browser->exec();
}

void MainWindow::ShowFIFOPlayer()
{
  if (!m_fifo_window)
  {
    m_fifo_window = new FIFOPlayerWindow(Core::System::GetInstance().GetFifoPlayer(),
                                         Core::System::GetInstance().GetFifoRecorder());
    connect(m_fifo_window, &FIFOPlayerWindow::LoadFIFORequested, this,
            [this](const QString& path) { StartGame(path, ScanForSecondDisc::No); });
  }

  SetQWidgetWindowDecorations(m_fifo_window);
  m_fifo_window->show();
  m_fifo_window->raise();
  m_fifo_window->activateWindow();
}

void MainWindow::ShowSkylanderPortal()
{
  if (!m_skylander_window)
  {
    m_skylander_window = new SkylanderPortalWindow();
  }

  SetQWidgetWindowDecorations(m_skylander_window);
  m_skylander_window->show();
  m_skylander_window->raise();
  m_skylander_window->activateWindow();
}

void MainWindow::ShowInfinityBase()
{
  if (!m_infinity_window)
  {
    m_infinity_window = new InfinityBaseWindow();
  }

  SetQWidgetWindowDecorations(m_infinity_window);
  m_infinity_window->show();
  m_infinity_window->raise();
  m_infinity_window->activateWindow();
}

void MainWindow::StateLoad()
{
  QString dialog_path = (Config::Get(Config::MAIN_CURRENT_STATE_PATH).empty()) ?
                            QDir::currentPath() :
                            QString::fromStdString(Config::Get(Config::MAIN_CURRENT_STATE_PATH));
  QString path = DolphinFileDialog::getOpenFileName(
      this, tr("Select a File"), dialog_path, tr("All Save States (*.sav *.s##);; All Files (*)"));
  Config::SetBase(Config::MAIN_CURRENT_STATE_PATH, QFileInfo(path).dir().path().toStdString());
  if (!path.isEmpty())
    State::LoadAs(Core::System::GetInstance(), path.toStdString());
}

void MainWindow::StateSave()
{
  QString dialog_path = (Config::Get(Config::MAIN_CURRENT_STATE_PATH).empty()) ?
                            QDir::currentPath() :
                            QString::fromStdString(Config::Get(Config::MAIN_CURRENT_STATE_PATH));
  QString path = DolphinFileDialog::getSaveFileName(
      this, tr("Select a File"), dialog_path, tr("All Save States (*.sav *.s##);; All Files (*)"));
  Config::SetBase(Config::MAIN_CURRENT_STATE_PATH, QFileInfo(path).dir().path().toStdString());
  if (!path.isEmpty())
    State::SaveAs(Core::System::GetInstance(), path.toStdString());
}

void MainWindow::StateLoadSlot()
{
  State::Load(Core::System::GetInstance(), m_state_slot);
}

void MainWindow::StateSaveSlot()
{
  State::Save(Core::System::GetInstance(), m_state_slot);
}

void MainWindow::StateLoadSlotAt(int slot)
{
  State::Load(Core::System::GetInstance(), slot);
}

void MainWindow::StateLoadLastSavedAt(int slot)
{
  State::LoadLastSaved(Core::System::GetInstance(), slot);
}

void MainWindow::StateSaveSlotAt(int slot)
{
  State::Save(Core::System::GetInstance(), slot);
}

void MainWindow::StateLoadUndo()
{
  State::UndoLoadState(Core::System::GetInstance());
}

void MainWindow::StateSaveUndo()
{
  State::UndoSaveState(Core::System::GetInstance());
}

void MainWindow::StateSaveOldest()
{
  State::SaveFirstSaved(Core::System::GetInstance());
}

void MainWindow::SetStateSlot(int slot)
{
  Settings::Instance().SetStateSlot(slot);
  m_state_slot = slot;

  Core::DisplayMessage(fmt::format("Selected slot {} - {}", m_state_slot,
                                   State::GetInfoStringOfSlot(m_state_slot, false)),
                       2500);
}

void MainWindow::IncrementSelectedStateSlot()
{
  u32 state_slot = m_state_slot + 1;
  if (state_slot > State::NUM_STATES)
    state_slot = 1;
  m_menu_bar->SetStateSlot(state_slot);
}

void MainWindow::DecrementSelectedStateSlot()
{
  u32 state_slot = m_state_slot - 1;
  if (state_slot < 1)
    state_slot = State::NUM_STATES;
  m_menu_bar->SetStateSlot(state_slot);
}

void MainWindow::PerformOnlineUpdate(const std::string& region)
{
  WiiUpdate::PerformOnlineUpdate(region, this);
  // Since the update may have installed a newer system menu, trigger a refresh.
  Settings::Instance().NANDRefresh();
}

void MainWindow::BootWiiSystemMenu()
{
  StartGame(std::make_unique<BootParameters>(BootParameters::NANDTitle{Titles::SYSTEM_MENU}));
}

void MainWindow::NetPlayInit()
{
  const auto& game_list_model = m_game_list->GetGameListModel();
  m_netplay_setup_dialog = new NetPlaySetupDialog(game_list_model, this);
  m_netplay_dialog = new NetPlayDialog(
      game_list_model,
      [this](const std::string& path, std::unique_ptr<BootSessionData> boot_session_data) {
        StartGame(path, ScanForSecondDisc::Yes, std::move(boot_session_data));
      });
#ifdef USE_DISCORD_PRESENCE
  m_netplay_discord = new DiscordHandler(this);
#endif

  connect(m_netplay_dialog, &NetPlayDialog::Stop, this, &MainWindow::ForceStop);
  connect(m_netplay_dialog, &NetPlayDialog::rejected, this, &MainWindow::NetPlayQuit);
  connect(m_netplay_setup_dialog, &NetPlaySetupDialog::Join, this, &MainWindow::NetPlayJoin);
  connect(m_netplay_setup_dialog, &NetPlaySetupDialog::Host, this, &MainWindow::NetPlayHost);
#ifdef USE_DISCORD_PRESENCE
  connect(m_netplay_discord, &DiscordHandler::Join, this, &MainWindow::NetPlayJoin);

  Discord::InitNetPlayFunctionality(*m_netplay_discord);
  m_netplay_discord->Start();
#endif
  connect(&Settings::Instance(), &Settings::ConfigChanged, this,
          &MainWindow::UpdateScreenSaverInhibition);
  connect(&Settings::Instance(), &Settings::EmulationStateChanged, this,
          &MainWindow::UpdateScreenSaverInhibition);
}

bool MainWindow::NetPlayJoin()
{
  if (Core::IsRunning())
  {
    ModalMessageBox::critical(nullptr, tr("Error"),
                              tr("Can't start a NetPlay Session while a game is still running!"));
    return false;
  }

  if (m_netplay_dialog->isVisible())
  {
    ModalMessageBox::critical(nullptr, tr("Error"),
                              tr("A NetPlay Session is already in progress!"));
    return false;
  }

  auto server = Settings::Instance().GetNetPlayServer();

  // Settings
  const std::string traversal_choice = Config::Get(Config::NETPLAY_TRAVERSAL_CHOICE);
  const bool is_traversal = traversal_choice == "traversal";

  std::string host_ip;
  u16 host_port;
  if (server)
  {
    host_ip = "127.0.0.1";
    host_port = server->GetPort();
  }
  else
  {
    host_ip = is_traversal ? Config::Get(Config::NETPLAY_HOST_CODE) :
                             Config::Get(Config::NETPLAY_ADDRESS);
    host_port = Config::Get(Config::NETPLAY_CONNECT_PORT);
  }

  const std::string traversal_host = Config::Get(Config::NETPLAY_TRAVERSAL_SERVER);
  const u16 traversal_port = Config::Get(Config::NETPLAY_TRAVERSAL_PORT);
  const std::string nickname = Config::Get(Config::NETPLAY_NICKNAME);
  const std::string network_mode = Config::Get(Config::NETPLAY_NETWORK_MODE);
  const bool host_input_authority = network_mode == "hostinputauthority" || network_mode == "golf";

  if (server)
  {
    server->SetHostInputAuthority(host_input_authority);
    server->AdjustPadBufferSize(Config::Get(Config::NETPLAY_BUFFER_SIZE));
  }

  // Create Client
  const bool is_hosting_netplay = server != nullptr;
  Settings::Instance().ResetNetPlayClient(new NetPlay::NetPlayClient(
      host_ip, host_port, m_netplay_dialog, nickname,
      NetPlay::NetTraversalConfig{is_hosting_netplay ? false : is_traversal, traversal_host,
                                  traversal_port}));

  if (!Settings::Instance().GetNetPlayClient()->IsConnected())
  {
    NetPlayQuit();
    return false;
  }

  m_netplay_setup_dialog->close();
  m_netplay_dialog->show(nickname, is_traversal);

  return true;
}

bool MainWindow::NetPlayHost(const UICommon::GameFile& game)
{
  if (Core::IsRunning())
  {
    ModalMessageBox::critical(nullptr, tr("Error"),
                              tr("Can't start a NetPlay Session while a game is still running!"));
    return false;
  }

  if (m_netplay_dialog->isVisible())
  {
    ModalMessageBox::critical(nullptr, tr("Error"),
                              tr("A NetPlay Session is already in progress!"));
    return false;
  }

  // Settings
  u16 host_port = Config::Get(Config::NETPLAY_HOST_PORT);
  const std::string traversal_choice = Config::Get(Config::NETPLAY_TRAVERSAL_CHOICE);
  const bool is_traversal = traversal_choice == "traversal";
  const bool use_upnp = Config::Get(Config::NETPLAY_USE_UPNP);

  const std::string traversal_host = Config::Get(Config::NETPLAY_TRAVERSAL_SERVER);
  const u16 traversal_port = Config::Get(Config::NETPLAY_TRAVERSAL_PORT);
  const u16 traversal_port_alt = Config::Get(Config::NETPLAY_TRAVERSAL_PORT_ALT);

  if (is_traversal)
    host_port = Config::Get(Config::NETPLAY_LISTEN_PORT);

  // Create Server
  Settings::Instance().ResetNetPlayServer(
      new NetPlay::NetPlayServer(host_port, use_upnp, m_netplay_dialog,
                                 NetPlay::NetTraversalConfig{is_traversal, traversal_host,
                                                             traversal_port, traversal_port_alt}));

  if (!Settings::Instance().GetNetPlayServer()->is_connected)
  {
    ModalMessageBox::critical(
        nullptr, tr("Failed to open server"),
        tr("Failed to listen on port %1. Is another instance of the NetPlay server running?")
            .arg(host_port));
    NetPlayQuit();
    return false;
  }

  Settings::Instance().GetNetPlayServer()->ChangeGame(game.GetSyncIdentifier(),
                                                      m_game_list->GetNetPlayName(game));

  // Join our local server
  return NetPlayJoin();
}

void MainWindow::NetPlayQuit()
{
  Settings::Instance().ResetNetPlayClient();
  Settings::Instance().ResetNetPlayServer();
#ifdef USE_DISCORD_PRESENCE
  Discord::UpdateDiscordPresence();
#endif
}

void MainWindow::UpdateScreenSaverInhibition()
{
  const bool inhibit =
      Config::Get(Config::MAIN_DISABLE_SCREENSAVER) && (Core::GetState() == Core::State::Running);

  if (inhibit == m_is_screensaver_inhibited)
    return;

  m_is_screensaver_inhibited = inhibit;

#ifdef HAVE_X11
  if (GetWindowSystemType() == WindowSystemType::X11)
    UICommon::InhibitScreenSaver(winId(), inhibit);
#else
  UICommon::InhibitScreenSaver(inhibit);
#endif
}

bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
  if (event->type() == QEvent::Close)
  {
    if (RequestStop() && object == this)
      m_exit_requested = true;

    static_cast<QCloseEvent*>(event)->ignore();
    return true;
  }

  return false;
}

QMenu* MainWindow::createPopupMenu()
{
  // Disable the default popup menu as it exposes the debugger UI even when the debugger UI is
  // disabled, which can lead to user confusion (see e.g. https://bugs.dolphin-emu.org/issues/13306)
  return nullptr;
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() == 1)
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{
  const QList<QUrl>& urls = event->mimeData()->urls();
  if (urls.empty())
    return;

  QStringList files;
  QStringList folders;

  for (const QUrl& url : urls)
  {
    QFileInfo file_info(url.toLocalFile());
    QString path = file_info.filePath();

    if (!file_info.exists() || !file_info.isReadable())
    {
      ModalMessageBox::critical(this, tr("Error"), tr("Failed to open '%1'").arg(path));
      return;
    }

    (file_info.isFile() ? files : folders).append(path);
  }

  if (!files.isEmpty())
  {
    StartGame(StringListToStdVector(files));
  }
  else
  {
    Settings& settings = Settings::Instance();
    const bool show_confirm = !settings.GetPaths().empty();

    for (const QString& folder : folders)
    {
      if (show_confirm)
      {
        if (ModalMessageBox::question(
                this, tr("Confirm"),
                tr("Do you want to add \"%1\" to the list of Game Paths?").arg(folder)) !=
            QMessageBox::Yes)
          return;
      }
      settings.AddPath(folder);
    }
  }
}

QSize MainWindow::sizeHint() const
{
  return QSize(800, 600);
}

#ifdef _WIN32
bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
  auto* msg = reinterpret_cast<MSG*>(message);
  if (msg && msg->message == WM_SETTINGCHANGE && msg->lParam != NULL &&
      std::wstring_view(L"ImmersiveColorSet")
              .compare(reinterpret_cast<const wchar_t*>(msg->lParam)) == 0)
  {
    // Windows light/dark theme has changed. Update our flag and refresh the theme.
    auto& settings = Settings::Instance();
    const bool was_dark_before = settings.IsSystemDark();
    settings.UpdateSystemDark();
    if (settings.IsSystemDark() != was_dark_before)
    {
      settings.ApplyStyle();

      // force the colors in the Skylander window to update
      if (m_skylander_window)
        m_skylander_window->RefreshList();
    }

    // TODO: When switching from light to dark, the window decorations remain light. Qt seems very
    // convinced that it needs to change these in response to this message, so even if we set them
    // to dark here, Qt sets them back to light afterwards.
  }

  return false;
}
#endif

void MainWindow::OnBootGameCubeIPL(DiscIO::Region region)
{
  StartGame(std::make_unique<BootParameters>(BootParameters::IPL{region}));
}

void MainWindow::OnImportNANDBackup()
{
  auto response = ModalMessageBox::question(
      this, tr("Question"),
      tr("Merging a new NAND over your currently selected NAND will overwrite any channels "
         "and savegames that already exist. This process is not reversible, so it is "
         "recommended that you keep backups of both NANDs. Are you sure you want to "
         "continue?"));

  if (response == QMessageBox::No)
    return;

  QString file =
      DolphinFileDialog::getOpenFileName(this, tr("Select the save file"), QDir::currentPath(),
                                         tr("BootMii NAND backup file (*.bin);;"
                                            "All Files (*)"));

  if (file.isEmpty())
    return;

  ParallelProgressDialog dialog(this);
  dialog.GetRaw()->setMinimum(0);
  dialog.GetRaw()->setMaximum(0);
  dialog.GetRaw()->setLabelText(tr("Importing NAND backup"));
  dialog.GetRaw()->setCancelButton(nullptr);

  auto beginning = QDateTime::currentDateTime().toMSecsSinceEpoch();

  std::future<void> result = std::async(std::launch::async, [&] {
    DiscIO::NANDImporter().ImportNANDBin(
        file.toStdString(),
        [&dialog, beginning] {
          dialog.SetLabelText(
              tr("Importing NAND backup\n Time elapsed: %1s")
                  .arg((QDateTime::currentDateTime().toMSecsSinceEpoch() - beginning) / 1000));
        },
        [this] {
          std::optional<std::string> keys_file = RunOnObject(this, [this] {
            return DolphinFileDialog::getOpenFileName(
                       this, tr("Select the keys file (OTP/SEEPROM dump)"), QDir::currentPath(),
                       tr("BootMii keys file (*.bin);;"
                          "All Files (*)"))
                .toStdString();
          });
          if (keys_file)
            return *keys_file;
          return std::string("");
        });
    dialog.Reset();
  });

  SetQWidgetWindowDecorations(dialog.GetRaw());
  dialog.GetRaw()->exec();

  result.wait();

  m_menu_bar->UpdateToolsMenu(Core::IsRunning());
}

void MainWindow::OnPlayRecording()
{
  QString dtm_file = DolphinFileDialog::getOpenFileName(
      this, tr("Select the Recording File to Play"), QString(), tr("Dolphin TAS Movies (*.dtm)"));

  if (dtm_file.isEmpty())
    return;

  auto& movie = Core::System::GetInstance().GetMovie();
  if (!movie.IsReadOnly())
  {
    // let's make the read-only flag consistent at the start of a movie.
    movie.SetReadOnly(true);
    emit ReadOnlyModeChanged(true);
  }

  std::optional<std::string> savestate_path;
  if (movie.PlayInput(dtm_file.toStdString(), &savestate_path))
  {
    emit RecordingStatusChanged(true);

    Play(savestate_path);
  }
}

void MainWindow::OnStartRecording()
{
  auto& movie = Core::System::GetInstance().GetMovie();
  if ((!Core::IsRunningAndStarted() && Core::IsRunning()) || movie.IsRecordingInput() ||
      movie.IsPlayingInput())
  {
    return;
  }

  if (movie.IsReadOnly())
  {
    // The user just chose to record a movie, so that should take precedence
    movie.SetReadOnly(false);
    emit ReadOnlyModeChanged(true);
  }

  Movie::ControllerTypeArray controllers{};
  Movie::WiimoteEnabledArray wiimotes{};

  for (int i = 0; i < 4; i++)
  {
    const SerialInterface::SIDevices si_device = Config::Get(Config::GetInfoForSIDevice(i));
    if (si_device == SerialInterface::SIDEVICE_GC_GBA_EMULATED)
      controllers[i] = Movie::ControllerType::GBA;
    else if (SerialInterface::SIDevice_IsGCController(si_device))
      controllers[i] = Movie::ControllerType::GC;
    else
      controllers[i] = Movie::ControllerType::None;
    wiimotes[i] = Config::Get(Config::GetInfoForWiimoteSource(i)) != WiimoteSource::None;
  }

  if (movie.BeginRecordingInput(controllers, wiimotes))
  {
    emit RecordingStatusChanged(true);

    if (!Core::IsRunning())
      Play();
  }
}

void MainWindow::OnStopRecording()
{
  auto& movie = Core::System::GetInstance().GetMovie();
  if (movie.IsRecordingInput())
    OnExportRecording();
  if (movie.IsMovieActive())
    movie.EndPlayInput(false);
  emit RecordingStatusChanged(false);
}

void MainWindow::OnExportRecording()
{
  auto& system = Core::System::GetInstance();
  const Core::CPUThreadGuard guard(system);

  QString dtm_file = DolphinFileDialog::getSaveFileName(
      this, tr("Save Recording File As"), QString(), tr("Dolphin TAS Movies (*.dtm)"));
  if (!dtm_file.isEmpty())
    system.GetMovie().SaveRecording(dtm_file.toStdString());
}

void MainWindow::OnActivateChat()
{
  if (g_netplay_chat_ui)
    g_netplay_chat_ui->Activate();
}

void MainWindow::OnRequestGolfControl()
{
  auto client = Settings::Instance().GetNetPlayClient();
  if (client)
    client->RequestGolfControl();
}

void MainWindow::ShowTASInput()
{
  for (int i = 0; i < num_gc_controllers; i++)
  {
    const auto si_device = Config::Get(Config::GetInfoForSIDevice(i));
    if (si_device == SerialInterface::SIDEVICE_GC_GBA_EMULATED)
    {
      SetQWidgetWindowDecorations(m_gba_tas_input_windows[i]);
      m_gba_tas_input_windows[i]->show();
      m_gba_tas_input_windows[i]->raise();
      m_gba_tas_input_windows[i]->activateWindow();
    }
    else if (si_device != SerialInterface::SIDEVICE_NONE &&
             si_device != SerialInterface::SIDEVICE_GC_GBA)
    {
      SetQWidgetWindowDecorations(m_gc_tas_input_windows[i]);
      m_gc_tas_input_windows[i]->show();
      m_gc_tas_input_windows[i]->raise();
      m_gc_tas_input_windows[i]->activateWindow();
    }
  }

  for (int i = 0; i < num_wii_controllers; i++)
  {
    if (Config::Get(Config::GetInfoForWiimoteSource(i)) == WiimoteSource::Emulated &&
        (!Core::IsRunning() || Core::System::GetInstance().IsWii()))
    {
      SetQWidgetWindowDecorations(m_wii_tas_input_windows[i]);
      m_wii_tas_input_windows[i]->show();
      m_wii_tas_input_windows[i]->raise();
      m_wii_tas_input_windows[i]->activateWindow();
    }
  }
}

void MainWindow::OnConnectWiiRemote(int id)
{
  const Core::CPUThreadGuard guard(Core::System::GetInstance());
  if (const auto bt = WiiUtils::GetBluetoothEmuDevice())
  {
    const auto wm = bt->AccessWiimoteByIndex(id);
    wm->Activate(!wm->IsConnected());
  }
}

//#ifdef USE_RETRO_ACHIEVEMENTS
//void MainWindow::ShowAchievementsWindow()
//{
//  if (!m_achievements_window)
//  {
//    m_achievements_window = new AchievementsWindow(this);
//  }
//
//  SetQWidgetWindowDecorations(m_achievements_window);
//  m_achievements_window->show();
//  m_achievements_window->raise();
//  m_achievements_window->activateWindow();
//}
//
//void MainWindow::ShowAchievementSettings()
//{
//  ShowAchievementsWindow();
//  m_achievements_window->ForceSettingsTab();
//}
//#endif  // USE_RETRO_ACHIEVEMENTS

void MainWindow::ShowMemcardManager()
{
  GCMemcardManager manager(this);

  SetQWidgetWindowDecorations(&manager);
  manager.exec();
}

void MainWindow::ShowResourcePackManager()
{
  ResourcePackManager manager(this);

  SetQWidgetWindowDecorations(&manager);
  manager.exec();
}

void MainWindow::ShowCheatsManager()
{
  SetQWidgetWindowDecorations(m_cheats_manager);
  m_cheats_manager->show();
}

void MainWindow::ShowRiivolutionBootWidget(const UICommon::GameFile& game)
{
  auto second_game = m_game_list->FindSecondDisc(game);
  std::vector<std::string> paths = {game.GetFilePath()};
  if (second_game != nullptr)
    paths.push_back(second_game->GetFilePath());
  std::unique_ptr<BootParameters> boot_params = BootParameters::GenerateFromFile(paths);
  if (!boot_params)
    return;
  if (!std::holds_alternative<BootParameters::Disc>(boot_params->parameters))
    return;

  auto& disc = std::get<BootParameters::Disc>(boot_params->parameters);
  RiivolutionBootWidget w(disc.volume->GetGameID(), disc.volume->GetRevision(),
                          disc.volume->GetDiscNumber(), game.GetFilePath(), this);
  SetQWidgetWindowDecorations(&w);

//#ifdef USE_RETRO_ACHIEVEMENTS
//  connect(&w, &RiivolutionBootWidget::OpenAchievementSettings, this,
//          &MainWindow::ShowAchievementSettings);
//#endif  // USE_RETRO_ACHIEVEMENTS

  w.exec();
  if (!w.ShouldBoot())
    return;

  AddRiivolutionPatches(boot_params.get(), std::move(w.GetPatches()));
  StartGame(std::move(boot_params));
}

void MainWindow::Show()
{
  if (!Settings::Instance().IsBatchModeEnabled())
  {
    SetQWidgetWindowDecorations(this);
    QWidget::show();
  }

  // If the booting of a game was requested on start up, do that now
  if (m_pending_boot != nullptr)
  {
    StartGame(std::move(m_pending_boot));
    m_pending_boot.reset();
  }
}
