// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// SPDX-FileCopyrightText: 2021 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

pragma Singleton

import QtQuick.Templates as T

import org.kde.union.impl as Union

T.Calendar {
	Union.Element.type: "Calendar"
	
	id: control
}
