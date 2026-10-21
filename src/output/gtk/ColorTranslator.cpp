// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#include "ColorTranslator.h"

#include <cmath>

using namespace Qt::StringLiterals;

namespace Union::Gtk
{

static QString formatNumber(qreal value)
{
    return QString::number(std::round(value * 1000.0) / 1000.0, 'g', 4);
}

std::optional<QColor> resolveColor(const std::optional<Union::Color> &color)
{
    const auto rgba = color ? color->toRgba() : Union::Color{};
    if (!rgba.isValid()) {
        return std::nullopt;
    }
    return rgba.toQColor();
}

QString toCssColor(const QColor &color)
{
    if (color.alpha() == 255) {
        return color.name(QColor::HexRgb);
    }
    return u"rgba(%1, %2, %3, %4)"_s.arg(color.red()).arg(color.green()).arg(color.blue()).arg(formatNumber(color.alphaF()));
}

std::optional<QString> toColorExpression(const std::optional<Union::Color> &color)
{
    if (auto resolved = resolveColor(color)) {
        return toCssColor(resolved.value());
    }
    return std::nullopt;
}
}
