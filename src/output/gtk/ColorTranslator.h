// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#pragma once

#include <optional>

#include <QColor>
#include <QString>

#include <Color.h>

namespace Union
{
namespace Gtk
{

std::optional<QColor> resolveColor(const std::optional<Union::Color> &color);
QString toCssColor(const QColor &color);

/*!
 * \brief Convert a colour to a GTK CSS colour expression.
 *
 * can only output dynamic colours for kcolorscheme mapped with kde-gtk-config
 */
std::optional<QString> toColorExpression(const std::optional<Union::Color> &color);
}
}
