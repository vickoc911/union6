// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

import QtQuick
import QtQuick.Templates as T
import org.kde.union.impl as Union

T.Action {
    id: control
    Union.Element.type: "Action"
    icon {
        width: Union.Style.properties.icon.width
        height: Union.Style.properties.icon.height
    }
}
