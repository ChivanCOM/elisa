/*
   SPDX-FileCopyrightText: 2016 (c) Matthieu Gallien <matthieu_gallien@yahoo.fr>

   SPDX-License-Identifier: LGPL-3.0-or-later
 */

import QtQuick
import QtQuick.Controls 2.4
import QtQml.Models 2.2
import org.kde.kirigami 2.10 as Kirigami
import org.kde.elisa

ScrollView {
    id: scrollView

    property alias model: viewModeView.model

    property alias viewIndex: viewModeView.currentIndex
    readonly property int wideWidth: Kirigami.Units.gridUnit * 12
    readonly property real iconsOnlyMinWidth: viewModeView.iconsOnlyWidth + (ScrollBar.vertical.visible ? ScrollBar.vertical.implicitWidth : 0)
    readonly property real iconsOnlyMaxWidth: viewModeView.iconsOnlyWidth + ScrollBar.vertical.implicitWidth

    signal switchView(int viewIndex)

    // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    contentItem: ListView {
        id: viewModeView

        readonly property int iconsOnlyWidth: Kirigami.Units.iconSizes.smallMedium + 2 * Kirigami.Units.largeSpacing

        reuseItems: true
        clip: true
        focus: true
        activeFocusOnTab: true
        keyNavigationEnabled: true
        interactive: true

        delegate: ItemDelegate {
            id: delegate

            width: viewModeView.width

            icon.source: model.image
            text: model.display
            highlighted: ListView.isCurrentItem

            // Prevent icon recoloring for styles that don't set `icon.color: "transparent"` by default
            // otherwise it applies a single-color mask above the entire album cover image
            Binding {
                when: !model.image.toString().startsWith("image://icon/")
                delegate.icon.color: "transparent"
            }

            onClicked: {
                switchView(index)
                forceActiveFocus()
            }
        }

        section.property: 'entryCategory'
        section.delegate: Loader {
            active: section !== "default" && opacity > 0
            height: item ? item.implicitHeight : 0
            sourceComponent: Kirigami.ListSectionHeader {
                text: section
                width: viewModeView.width
            }
            opacity: scrollView.width > scrollView.iconsOnlyMaxWidth ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    easing.type: Easing.InOutQuad
                    duration: Kirigami.Units.longDuration
                }
            }
        }
    }
}
