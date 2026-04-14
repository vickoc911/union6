// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// SPDX-FileCopyrightText: 2025 Arjen Hiemstra <ahiemstra@heimr.nl>

import QtQuick
import QtQuick.Templates as T

import org.kde.union.impl as Union

Item {
    id: root

    required property T.AbstractButton control

    property int wrapMode: Text.NoWrap

    Union.PositionedItem.positionChildren: true
    Union.MnemonicData.enabled: root.control.enabled && root.control.visible
    Union.MnemonicData.controlType: Union.MnemonicData.ActionElement
    Union.MnemonicData.label: root.control.display !== T.AbstractButton.IconOnly ? root.control.text : ""
    Union.MnemonicData.onActiveChanged: root.control.background?.updateItem()

    Union.Icon {
        Union.PositionedItem.source: Union.PositionerSource.Icon
        control: root.control
        visible: (name || source.toString()) && root.control.display != T.AbstractButton.TextOnly
    }

    Text {
        Union.PositionedItem.source: Union.PositionerSource.Text

        text: root.Union.MnemonicData.richTextLabel ?? root.control.text
        font: root.control.font
        color: Union.Style.properties.text.color ?? "black"
        elide: root.wrapMode == Text.NoWrap ? Text.ElideRight : Text.ElideNone
        wrapMode: root.wrapMode

        visible: root.control.display != T.AbstractButton.IconOnly && text.length > 0
    }
}
