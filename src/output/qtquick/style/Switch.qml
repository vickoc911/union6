// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl as QQCImpl

import org.kde.union.impl as Union

T.Switch {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            Union.Positioner.implicitWidth)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             Union.Positioner.implicitHeight)

    Union.Element.type: "Switch"
    Union.Element.states {
        hovered: control.hovered
        activeFocus: control.activeFocus
        visualFocus: control.visualFocus
        pressed: control.down
        checked: control.checked
        enabled: control.enabled
        highlighted: control.highlighted
    }

    leftPadding: Union.Positioner.padding.left
    rightPadding: Union.Positioner.padding.right
    topPadding: Union.Positioner.padding.top
    bottomPadding: Union.Positioner.padding.bottom

    leftInset: Union.Style.properties.layout.inset.left
    rightInset: Union.Style.properties.layout.inset.right
    topInset: Union.Style.properties.layout.inset.top
    bottomInset: Union.Style.properties.layout.inset.bottom

    spacing: Union.Style.properties.layout.spacing

    icon {
        color: palette.buttonText
        width: Union.Style.properties.icon.width
        height: Union.Style.properties.icon.height
        name: Union.Style.properties.icon.name
        source: Union.Style.properties.icon.source
    }

    Union.Positioner.positionItems: [indicator, contentItem]

    indicator: T.Control {
        Union.Element.type: "Indicator"
        Union.Element.states {
            hovered: control.hovered
            activeFocus: control.activeFocus
            visualFocus: control.visualFocus
            pressed: control.down
            checked: control.checked
            enabled: control.enabled
            highlighted: control.highlighted
        }

        contentItem: Union.StyledRectangle { }
        background: Union.StyledRectangle { }
    }

    contentItem: Item {
        Union.PositionedItem.positionChildren: true

        QQCImpl.IconImage {
            Union.PositionedItem.source: Union.PositionerSource.Icon
            sourceSize.width: control.icon.width
            sourceSize.height: control.icon.height
            name: control.icon.name
            source: control.icon.source
            color: control.icon.color
            visible: control.display !== T.AbstractButton.TextOnly
        }
        Text {
            Union.PositionedItem.source: Union.PositionerSource.Text
            text: control.text
            font: control.font
            color: control.palette.text
            renderType: Text.NativeRendering
            visible: control.display !== T.AbstractButton.IconOnly
        }
    }
}
