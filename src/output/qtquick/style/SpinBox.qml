// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick
import QtQuick.Controls.impl as QQCImpl
import QtQuick.Templates as T

import org.kde.union.impl as Union

T.SpinBox {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentItem.implicitWidth + leftPadding + rightPadding + up.implicitIndicatorWidth + down.implicitIndicatorWidth)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             up.implicitIndicatorHeight, down.implicitIndicatorHeight)

    hoverEnabled: true

    Union.Element.type: "SpinBox"
    Union.Element.states {
        hovered: control.hovered
        activeFocus: control.activeFocus
        visualFocus: control.visualFocus
        enabled: control.enabled
    }
    Union.Element.hints: control.editable ? ["editable"] : []

    leftPadding: Union.Style.properties.layout.padding.left
    rightPadding: Union.Style.properties.layout.padding.right
    topPadding: Union.Style.properties.layout.padding.top
    bottomPadding: Union.Style.properties.layout.padding.bottom

    leftInset: Union.Style.properties.layout.inset.left
    rightInset: Union.Style.properties.layout.inset.right
    topInset: Union.Style.properties.layout.inset.top
    bottomInset: Union.Style.properties.layout.inset.bottom

    font: Union.Style.properties.text.font

    validator: IntValidator {
        locale: control.locale.name
        bottom: Math.min(control.from, control.to)
        top: Math.max(control.from, control.to)
    }

    contentItem: Union.Positioner {
        container: control

        implicitWidth: textField.implicitWidth
        implicitHeight: textField.implicitHeight

        T.TextField {
            id: textField

            Union.Positioner.source: Union.PositionerSource.Text
            Union.Positioner.horizontalAlignment: Union.Alignment.Fill
            Union.Positioner.verticalAlignment: Union.Alignment.Fill

            implicitWidth: contentWidth
            implicitHeight: contentHeight

            text: control.displayText
            font: control.font

            horizontalAlignment: Union.Alignment.toQtHorizontal(Union.Style.properties.text.alignment.horizontal)
            verticalAlignment: Union.Alignment.toQtVertical(Union.Style.properties.text.alignment.vertical)

            readOnly: !control.editable
            validator: control.validator
            inputMethodHints: control.inputMethodHints
            selectByMouse: true
            hoverEnabled: false
        }
    }

    up.indicator: Union.PositionedItem {
        Union.Element.type: "Indicator"
        Union.Element.hints: ["increase"]

        Union.Element.states {
            hovered: control.up.hovered
            pressed: control.up.pressed
        }

        container: control

        Union.StyledRectangle {
            anchors.fill: parent
            QQCImpl.IconImage {
                anchors.centerIn: parent
                width: Union.Style.properties.icon.width
                height: Union.Style.properties.icon.height
                name: Union.Style.properties.icon.name
                color: Union.Style.properties.icon.color
            }
        }
    }

    down.indicator: Union.PositionedItem {
        Union.Element.type: "Indicator"
        Union.Element.hints: ["decrease"]

        Union.Element.states {
            hovered: control.down.hovered
            pressed: control.down.pressed
        }

        container: control

        Union.StyledRectangle {
            anchors.fill: parent
            QQCImpl.IconImage {
                anchors.centerIn: parent
                width: Union.Style.properties.icon.width
                height: Union.Style.properties.icon.height
                name: Union.Style.properties.icon.name
                color: Union.Style.properties.icon.color
            }
        }
    }

    background: Union.StyledRectangle { }
}
