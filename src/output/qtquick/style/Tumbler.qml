// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import org.kde.union.impl as Union

T.Tumbler {
	id: control
	Union.Element.type: "Tumbler"
	Union.Element.states {
		hovered: control.hovered
		activeFocus: control.activeFocus
		visualFocus: control.visualFocus
		enabled: control.enabled
	}

	implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
							implicitContentWidth + leftPadding + rightPadding)
	implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
							 implicitContentHeight + topPadding + bottomPadding)


	leftPadding: Union.Style.properties.layout.padding.left
	rightPadding: Union.Style.properties.layout.padding.right
	topPadding: Union.Style.properties.layout.padding.top
	bottomPadding:  Union.Style.properties.layout.padding.bottom

	leftInset: Union.Style.properties.layout.inset.left
	rightInset: Union.Style.properties.layout.inset.right
	topInset: Union.Style.properties.layout.inset.top
	bottomInset: Union.Style.properties.layout.inset.bottom

	font: Union.Style.properties.text.font

	spacing: Union.Style.properties.layout.spacing

	readonly property real __delegateHeight: availableHeight / visibleItemCount

	delegate: Text {
		text: modelData
		font: control.font
		color: control.Union.Style.properties.text.color ?? "black"
		horizontalAlignment: control.Union.Alignment.toQtHorizontal(control.Union.Style.properties.text.alignment.horizontal)
		verticalAlignment: control.Union.Alignment.toQtVertical(control.Union.Style.properties.text.alignment.vertical)
		// TODO: better way to calculate the opacity here so that user can modify it?
		opacity: 1.0 - Math.abs(Tumbler.displacement) / (control.visibleItemCount / 2)
		// We use required property here to satisfy qmllint, but that means
		// we also need to declare the index for the attached properties
		// (see QQuickTumblerAttachedPrivate::init).
		required property var modelData
		required property int index
	}

	contentItem: TumblerView {
		implicitWidth: control.Union.Style.properties.layout.width
		implicitHeight: control.Union.Style.properties.layout.height
		model: control.model
		delegate: control.delegate
		path: Path {
			startX: control.contentItem.width / 2
			startY: -control.__delegateHeight / 2

			PathLine {
				x: control.contentItem.width / 2
				y: (control.visibleItemCount + 1) * control.__delegateHeight - control.__delegateHeight / 2
			}
		}
	}

	background: Union.StyledRectangle { }
}
