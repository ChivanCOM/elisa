/*
   SPDX-FileCopyrightText: 2016 (c) Matthieu Gallien <matthieu_gallien@yahoo.fr>
   SPDX-FileCopyrightText: 2018 (c) Alexander Stippich <a.stippich@gmx.net>

   SPDX-License-Identifier: LGPL-3.0-or-later
 */

import QtQuick 2.7
import QtQuick.Window 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.0
import org.kde.elisa
import org.kde.kirigami 2.5 as Kirigami

Menu {
    id: applicationMenu
    title: i18nc("@title:menu", "Application Menu")

    property var reportBugAction: ElisaApplication.action("help_report_bug")
    property var aboutAppAction: ElisaApplication.action("help_about_app")
    property var aboutKdeAction: ElisaApplication.action("help_about_kde")
    property var configureShortcutsAction: ElisaApplication.action("options_configure_keybinding")
    property var configureAction: ElisaApplication.action("options_configure")
    property var quitAction: ElisaApplication.action("file_quit")

    Action {
        text: mainWindow.visibility == Window.FullScreen ? i18nc("@action:inmenu", "Exit Full Screen") : i18nc("@action:inmenu", "Enter Full Screen")
        icon.name: mainWindow.visibility == Window.FullScreen ? "view-restore" : "view-fullscreen"
        onTriggered: mainWindow.visibility == Window.FullScreen ? mainWindow.restorePreviousStateBeforeFullScreen() : mainWindow.goFullScreen()
    }

    MenuSeparator {}

     Menu {
        title: i18nc("@action:inmenu this has child menu items", "Render to")
        enabled: ElisaApplication.renderersSupported
        MenuItem {
                text: "None"
                checkable: true
                checked: true
                autoExclusive: true
                onTriggered: {
                     ElisaApplication.setRenderer("Local", "None");
                    applicationMenu.close();
                }
            }
        Repeater {
            model: ElisaApplication.renderersModel
            delegate: MenuItem {
                text: model.name
                checkable: true
                visible: model.type !== "None" 
                autoExclusive: true
                onTriggered: {
                    ElisaApplication.setRenderer(model.name, model.type);
                    applicationMenu.close();
                }
            }
        }
    }

    MenuSeparator {}

    Action {
        text: i18nc("@action:inmenu", "Scan for New Music")
        icon.name: "view-refresh"
        onTriggered: {
            applicationMenu.close()
            ElisaApplication.musicManager.scanCollection(MusicListenersManager.Soft)
        }
    }

    Action {
        text: i18nc("@action:inmenu", "Reset Database and Re-Scan Everything")
        icon.name: "edit-clear-all"
        onTriggered: {
            applicationMenu.close()
            ElisaApplication.musicManager.scanCollection(MusicListenersManager.Hard)
        }
    }

    MenuSeparator {}

    Menu {
        title: i18nc("@action:inmenu this has child menu items", "Color Scheme")
        Repeater {
            model: ElisaApplication.colorSchemesModel
            delegate: MenuItem {
                required property var model

                icon.source: "image://colorScheme/" + model.display
                icon.color: "transparent"
                text: model.display
                checkable: true
                checked: model.display === ElisaConfigurationDialog.colorScheme
                autoExclusive: true
                onTriggered: {
                    ElisaApplication.activateColorScheme(model.display)
                    ElisaConfigurationDialog.setColorScheme(model.display)
                    ElisaConfigurationDialog.save()
                    applicationMenu.close()
                }
            }
        }
    }

    Action {
        text: configureAction.text
        shortcut: ElisaApplication.actionShortcut(configureAction)
        icon.name: ElisaApplication.iconName(configureAction.icon)
        onTriggered: {
            applicationMenu.close()
            configureAction.trigger()
        }
    }

    Action {
        text: configureShortcutsAction.text
        shortcut: ElisaApplication.actionShortcut(configureShortcutsAction)
        icon.name: ElisaApplication.iconName(configureShortcutsAction.icon)
        onTriggered: {
            applicationMenu.close()
            configureShortcutsAction.trigger()
        }
    }

    MenuSeparator {}

    Action {
        text: aboutAppAction.text
        shortcut: ElisaApplication.actionShortcut(aboutAppAction)
        icon.name: ElisaApplication.iconName(aboutAppAction.icon)
        onTriggered: {
            applicationMenu.close()
            aboutAppAction.trigger()
        }
    }

    Action {
        text: aboutKdeAction.text
        shortcut: ElisaApplication.actionShortcut(aboutKdeAction)
        icon.name: ElisaApplication.iconName(aboutKdeAction.icon)
        onTriggered: {
            applicationMenu.close()
            aboutKdeAction.trigger()
        }
    }

    Action {
        text: reportBugAction.text
        shortcut: ElisaApplication.actionShortcut(reportBugAction)
        icon.name: ElisaApplication.iconName(reportBugAction.icon)
        onTriggered: {
            applicationMenu.close()
            reportBugAction.trigger()
        }
    }

    MenuSeparator {
        visible: ElisaApplication.showSystemTrayIcon
    }

    MenuItem {
        action: Action {
            text: quitAction.text
            shortcut: ElisaApplication.actionShortcut(quitAction)
            icon.name: ElisaApplication.iconName(quitAction.icon)
            onTriggered: {
                applicationMenu.close()
                quitAction.trigger()
            }
        }
        visible: ElisaApplication.showSystemTrayIcon
    }
}
