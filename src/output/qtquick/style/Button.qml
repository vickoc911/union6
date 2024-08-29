/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2022 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls.impl as QQCImpl
import QtQuick.Templates as T
import org.kde.union.impl as Union

T.Button {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: true

    Union.Element.type: "Button"
    Union.Element.colorSet: Union.ColorSet.Button
    Union.Element.states {
        hovered: control.hovered
        activeFocus: control.activeFocus
        visualFocus: control.visualFocus
        pressed: control.down
        checked: control.checked
        enabled: control.enabled
        highlighted: control.highlighted
    }
    Union.Element.hints: icon.name || icon.source.toString() ? ["with-icon"] : []

    leftPadding: Union.Style.properties.layout.padding.left
    rightPadding: Union.Style.properties.layout.padding.right
    topPadding: Union.Style.properties.layout.padding.top
    bottomPadding: Union.Style.properties.layout.padding.bottom

    leftInset: Union.Style.properties.layout.margins.left
    rightInset: Union.Style.properties.layout.margins.right
    topInset: Union.Style.properties.layout.margins.top
    bottomInset: Union.Style.properties.layout.margins.bottom

    font: Union.Style.properties.text.font

    spacing: Union.Style.properties.layout.spacing

    icon {
        color: Union.Style.properties.icon.color
        width: Union.Style.properties.icon.width
        height: Union.Style.properties.icon.height
        name: Union.Style.properties.icon.name
        source: Union.Style.properties.icon.source
    }

    contentItem: Union.Positioner {
        container: control

        Rectangle {
            Union.Positioner.source: Union.PositionerSource.Icon
            width: control.icon.width
            height: control.icon.height
            color: Qt.rgba(1, 0, 0, 0.1)

            QQCImpl.IconImage {
                anchors.fill: parent
                name: control.icon.name
                color: control.icon.color
            }
        }

        Text {
            Union.Positioner.source: Union.PositionerSource.Text

            text: control.text
            font: control.font
            color: Union.Style.properties.text.color
        }
    }

    background: Union.StyledRectangle { }
}
