// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2025 Akseli Lahtinen <akselmo@akselmo.dev>
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import org.kde.union.impl as Union

T.ComboBox {
    id: control

    Union.Element.type: "ComboBox"
    Union.Element.states {
        hovered: control.hovered
        activeFocus: control.activeFocus
        visualFocus: control.visualFocus
        pressed: control.down
        enabled: control.enabled
        highlighted: control.highlighted
    }
    Union.Element.hints: {
        let result = [];
        if (flat) {
            result.push("flat");
        }
        return result;
    }

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding, implicitIndicatorHeight + topPadding + bottomPadding)

    leftPadding: Union.Positioner.padding.left + (!control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)
    rightPadding: Union.Positioner.padding.right + (control.mirrored || !indicator || !indicator.visible ? 0 : indicator.width + spacing)
    topPadding: Union.Positioner.padding.top
    bottomPadding: Union.Positioner.padding.bottom

    font: Union.Style.properties.text.font
    spacing: Union.Style.properties.layout.spacing
    palette: Union.Style.properties.palette.quickPalette

    delegate: ItemDelegate {
        required property var model
        required property int index

        width: ListView.view.width
        text: model[control.textRole]
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled
    }

    indicator: IconImage {
        x: control.mirrored ? Union.Positioner.padding.left : control.width - width - Union.Positioner.padding.right
        y: control.topPadding + (control.availableHeight - height) / 2
        Union.PositionedItem.source: Union.PositionerSource.Icon
        name: Union.Style.properties.icon.name ?? ""
        palette: control.palette
        color: palette.buttonText
    }

    contentItem: TextField {
        text: control.editable ? control.editText : control.displayText
        padding: 0
        enabled: control.editable
        autoScroll: control.editable
        readOnly: control.down
        inputMethodHints: control.inputMethodHints
        validator: control.validator
        selectByMouse: control.selectTextByMouse
        background: Item {}
    }

    background: Union.StyledRectangle {
        anchors.fill: parent
    }

    popup: Popup {
        y: control.height
        width: control.width
        height: Math.min(contentItem.implicitHeight, control.Window.height)

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.delegateModel
            currentIndex: control.highlightedIndex
            highlightMoveDuration: 0
            T.ScrollIndicator.vertical: ScrollIndicator {}
        }
    }
}
