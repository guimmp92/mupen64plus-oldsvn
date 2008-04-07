/*
* Copyright (C) 2008 Louai Al-Khanji
*
* This program is free software; you can redistribute it and/
* or modify it under the terms of the GNU General Public Li-
* cence as published by the Free Software Foundation; either
* version 2 of the Licence, or any later version.
*
* This program is distributed in the hope that it will be use-
* ful, but WITHOUT ANY WARRANTY; without even the implied war-
* ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public Licence for more details.
*
* You should have received a copy of the GNU General Public
* Licence along with this program; if not, write to the Free
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
* USA.
*
*/

#include <KApplication>
#include <KStandardAction>
#include <KActionCollection>
#include <KAction>
#include <KLocale>
#include <KStatusBar>
#include <KFileDialog>
#include <KMessageBox>
#include <KRecentFilesAction>
#include <KConfigDialog>
#include <KDebug>

#include "mainwindow.h"
#include "mainwidget.h"
#include "globals.h"
#include "rommodel.h"
#include "plugins.h"

// Autogenerated files
#include "settings.h"
#include "ui_mainsettingswidget.h"
#include "ui_pluginssettingswidget.h"
#include "ui_rombrowsersettingswidget.h"

namespace core {
    extern "C" {
        #include "../main.h"
    }
}

enum StatusBarFields { ItemCountField };

MainWindow::MainWindow()
    : KXmlGuiWindow()
    , m_mainWidget(0)
    , m_actionRecentFiles(0)
{
    m_mainWidget = new MainWidget(this);
    setCentralWidget(m_mainWidget);
    createActions();
    setMinimumSize(640, 480); // hack, we should set proper size hints
    statusBar()->insertPermanentItem("", ItemCountField);

    connect(m_mainWidget, SIGNAL(itemCountChanged(int)),
             this, SLOT(updateItemCount(int)));
    connect(m_mainWidget, SIGNAL(romActivated(KUrl)),
             this, SLOT(romOpen(KUrl)));

    setupGUI();
}

void MainWindow::showInfoMessage(const QString& msg)
{
    statusBar()->showMessage(msg);
}

void MainWindow::showAlertMessage(const QString& msg)
{
    KMessageBox::error(this, msg);
}

bool MainWindow::confirmMessage(const QString& msg)
{
    switch(KMessageBox::questionYesNo(this, msg)) {
        case KMessageBox::Yes:
            return true;
            break;
        default:
            return false;
            break;
    }
}

void MainWindow::romOpen()
{
    QString filter = RomExtensions.join(" ");
    QString filename = KFileDialog::getOpenFileName(KUrl(), filter);
    if (!filename.isNull()) {
        romOpen(filename);
    }
}

void MainWindow::romOpen(const KUrl& url)
{
    QString path = url.path();
    if (url.isLocalFile()) {
        m_actionRecentFiles->addUrl(url);
        m_actionRecentFiles->saveEntries(KGlobal::config()->group("Recent Roms"));
        KGlobal::config()->sync();
        core::open_rom(path.toLocal8Bit());
        core::startEmulation();
    }
}

void MainWindow::romClose()
{
    core::close_rom();
}

void MainWindow::emulationStart()
{
    core::startEmulation();
}

void MainWindow::emulationPauseContinue()
{
    core::pauseContinueEmulation();
}

void MainWindow::emulationStop()
{
    core::stopEmulation();
}

// FIXME these need implementing...
void MainWindow::viewFullScreen() {}
void MainWindow::saveStateSave() {}
void MainWindow::saveStateSaveAs() {}
void MainWindow::saveStateRestore() {}
void MainWindow::saveStateLoad() {}

void MainWindow::configDialogShow()
{
    if (KConfigDialog::showDialog("settings")) {
        return;
    }

    KConfigDialog* dialog = new KConfigDialog(this, "settings",
                                              Settings::self());

    // Main settings
    QWidget* mainSettingsWidget = new QWidget(dialog);
    Ui::MainSettings().setupUi(mainSettingsWidget);
    dialog->addPage(mainSettingsWidget, i18n("Main Settings"),
                     "preferences-system");

    // Plugin Settings
    QWidget* pluginsSettingsWidget = new QWidget(dialog);
    Plugins* plugins = Plugins::self();
    QSize labelIconSize(32, 32);
    Ui::PluginsSettings pluginsSettingsUi;

    pluginsSettingsUi.setupUi(pluginsSettingsWidget);

    // gfx plugin
    pluginsSettingsUi.kcfg_GraphicsPlugin->addItems(
        plugins->plugins(Plugins::Graphics)
    );
    pluginsSettingsUi.graphicsPluginIconLabel->setPixmap(
        KIcon("applications-graphics").pixmap(labelIconSize)
    );
    connect(pluginsSettingsUi.aboutGraphicsPluginButton, SIGNAL(clicked()),
             plugins, SLOT(aboutGraphicsPlugin()));
    connect(pluginsSettingsUi.configureGraphicsPluginButton, SIGNAL(clicked()),
             plugins, SLOT(configureGraphicsPlugin()));
    connect(pluginsSettingsUi.testGraphicsPluginButton, SIGNAL(clicked()),
             plugins, SLOT(testGraphicsPlugin()));

    // audio plugin
    pluginsSettingsUi.kcfg_AudioPlugin->addItems(
        plugins->plugins(Plugins::Audio)
    );
    pluginsSettingsUi.audioPluginIconLabel->setPixmap(
        KIcon("audio-headset").pixmap(labelIconSize)
    );
    connect(pluginsSettingsUi.aboutAudioPluginButton, SIGNAL(clicked()),
             plugins, SLOT(aboutAudioPlugin()));
    connect(pluginsSettingsUi.configureAudioPluginButton, SIGNAL(clicked()),
             plugins, SLOT(configureAudioPlugin()));
    connect(pluginsSettingsUi.testAudioPluginButton, SIGNAL(clicked()),
             plugins, SLOT(testAudioPlugin()));

    // input plugin
    pluginsSettingsUi.kcfg_InputPlugin->addItems(
        plugins->plugins(Plugins::Input)
    );
    pluginsSettingsUi.inputPluginIconLabel->setPixmap(
        KIcon("input-gaming").pixmap(labelIconSize)
    );
    connect(pluginsSettingsUi.aboutInputPluginButton, SIGNAL(clicked()),
             plugins, SLOT(aboutInputPlugin()));
    connect(pluginsSettingsUi.configureInputPluginButton, SIGNAL(clicked()),
             plugins, SLOT(configureInputPlugin()));
    connect(pluginsSettingsUi.testInputPluginButton, SIGNAL(clicked()),
             plugins, SLOT(testInputPlugin()));

    // rsp plugin
    pluginsSettingsUi.kcfg_RspPlugin->addItems(
        plugins->plugins(Plugins::Rsp)
    );
    pluginsSettingsUi.rspPluginIconLabel->setPixmap(
        KIcon("cpu").pixmap(labelIconSize)
    );
    connect(pluginsSettingsUi.aboutRspPluginButton, SIGNAL(clicked()),
             plugins, SLOT(aboutRspPlugin()));
    connect(pluginsSettingsUi.configureRspPluginButton, SIGNAL(clicked()),
             plugins, SLOT(configureRspPlugin()));
    connect(pluginsSettingsUi.testRspPluginButton, SIGNAL(clicked()),
             plugins, SLOT(testRspPlugin()));

    dialog->addPage(pluginsSettingsWidget, plugins, i18n("Plugins"),
                     "applications-engineering");

    // Rom browser settings
    QWidget* romBrowserSettingsWidget = new QWidget(dialog);
    Ui::RomBrowserSettings().setupUi(romBrowserSettingsWidget);
    dialog->addPage(romBrowserSettingsWidget, i18n("Rom Browser"),
                     "preferences-system-network");

    connect(dialog, SIGNAL(settingsChanged(QString)),
             RomModel::self(), SLOT(settingsChanged()));

    dialog->show();
}

void MainWindow::updateItemCount(int count)
{
    statusBar()->changeItem(i18n("%0 roms").arg(count), ItemCountField);
}

void MainWindow::createActions()
{
    KAction *act = 0;

    // "Rom" menu
    KStandardAction::open(this, SLOT(romOpen()), actionCollection());
    m_actionRecentFiles = KStandardAction::openRecent(this, SLOT(romOpen(KUrl)),
                                                       actionCollection());
    m_actionRecentFiles->loadEntries(KGlobal::config()->group("Recent Roms"));
    KStandardAction::close(this, SLOT(romClose()), actionCollection());
    KStandardAction::quit(KApplication::instance(), SLOT(quit()),
                          actionCollection());

    // "Emulation" menu
    act = actionCollection()->addAction("emulation_start");
    act->setText(i18n("Start"));
    act->setIcon(KIcon("media-playback-start"));
    connect(act, SIGNAL(triggered()), this, SLOT(emulationStart()));
    
    act = actionCollection()->addAction("emulation_pause_continue");
    act->setText(i18n("Pause"));
    act->setIcon(KIcon("media-playback-pause"));
    act->setCheckable(true);
    connect(act, SIGNAL(triggered()), this, SLOT(emulationPauseContinue()));
    
    act = actionCollection()->addAction("emulation_stop");
    act->setText(i18n("Stop"));
    act->setIcon(KIcon("media-playback-stop"));
    connect(act, SIGNAL(triggered()), this, SLOT(emulationStop()));

    act = actionCollection()->addAction("fullscreen");
    act->setText(i18n("Full Screen"));
    act->setIcon(KIcon("view-fullscreen"));
    connect(act, SIGNAL(triggered()), this, SLOT(viewFullScreen()));

    act = actionCollection()->addAction("save_state_save");
    act->setText(i18n("Save State"));
    act->setIcon(KIcon("document-save"));
    connect(act, SIGNAL(triggered()), this, SLOT(saveStateSave()));
    
    act = actionCollection()->addAction("save_state_save_as");
    act->setText(i18n("Save State as..."));
    act->setIcon(KIcon("document-save-as"));
    connect(act, SIGNAL(triggered()), this, SLOT(saveStateSaveAs()));
    
    act = actionCollection()->addAction("save_state_restore");
    act->setText(i18n("Restore State"));
    act->setIcon(KIcon("document-revert"));
    connect(act, SIGNAL(triggered()), this, SLOT(saveStateRestore()));
    
    act = actionCollection()->addAction("save_state_load");
    act->setText(i18n("Load State"));
    act->setIcon(KIcon("document-open"));
    connect(act, SIGNAL(triggered()), this, SLOT(saveStateLoad()));
    
    // Other stuff
    KStandardAction::preferences(this, SLOT(configDialogShow()),
                                  actionCollection());
}

#include "mainwindow.moc"
