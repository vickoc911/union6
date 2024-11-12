// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later

import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl as QQCImpl

T.CheckDelegate {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            Union.Positioner.implicitWidth)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding,
                             Union.Positioner.implicitHeight)

    Union.Element.type: "CheckDelegate"
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

    leftPadding: Union.Positioner.padding.left
    rightPadding: Union.Positioner.padding.right
    topPadding: Union.Positioner.padding.top
    bottomPadding: Union.Positioner.padding.bottom

    leftInset: Union.Style.properties.layout.inset.left
    rightInset: Union.Style.properties.layout.inset.right
    topInset: Union.Style.properties.layout.inset.top
    bottomInset: Union.Style.properties.layout.inset.bottom

    font: Union.Style.properties.text.font

    spacing: Union.Style.properties.layout.spacing

    icon {
        color: Union.Style.properties.icon.color
        width: Union.Style.properties.icon.width
        height: Union.Style.properties.icon.height
        name: Union.Style.properties.icon.name
        source: Union.Style.properties.icon.source
    }

    Union.Positioner.positionItems: [
        contentItem,
        indicator
    ]

    indicator: Union.StyledRectangle {
        Union.Element.type: "Indicator"
        Union.Element.hints: ["check-delegate"]
    }

    contentItem: Item {
        Union.PositionedItem.positionChildren: true
        QQCImpl.IconImage {
            Union.PositionedItem.source: Union.PositionerSource.Icon
            width: control.icon.width
            height: control.icon.height
            name: control.icon.name
            color: control.icon.color
            visible: name.length > 0
        }
        QQCImpl.MnemonicLabel {
            Union.PositionedItem.source: Union.PositionerSource.Text
            text: control.text
            font: control.font
            color: Union.Style.properties.text.color
            renderType: Text.NativeRendering
        }
    }

    background: Union.StyledRectangle {}
}
