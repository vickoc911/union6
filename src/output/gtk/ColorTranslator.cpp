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

// FIXME: GTK 4 has modern css colour support which can reduce the amount of fixedColor()
std::optional<QString> toColorExpression(const std::optional<Union::Color> &color)
{
    if (!color || !color->isValid()) {
        return std::nullopt;
    }

    auto fixedColor = [&color]() -> std::optional<QString> {
        if (auto resolved = resolveColor(color)) {
            return toCssColor(resolved.value());
        }
        return std::nullopt;
    };

    switch (color->type()) {
    case Union::ColorData::Type::Custom: {
        // frame contrast is a number, not a colour
        const auto arguments = color->customArguments();
        if (color->customSource() != u"kcolorscheme"_s || arguments.size() != 4 || arguments.at(3).compare(u"framecontrast", Qt::CaseInsensitive) == 0) {
            return fixedColor();
        }
        return u"@"_s + arguments.join(u'_').toLower();
    }
    case Union::ColorData::Type::MixOperation: {
        const auto operands = color->operands();
        const auto first = toColorExpression(operands.value(0));
        const auto second = toColorExpression(operands.value(1));
        if (!first || !second) {
            return fixedColor();
        }
        return u"mix(%1, %2, %3)"_s.arg(first.value(), second.value(), formatNumber(color->mixAmount()));
    }
    case Union::ColorData::Type::SetOperation: {
        // GTK can only do this when just the alpha is replaced. alpha() multiplies
        // so the factor has to be worked out from the base colour
        const auto components = color->setComponents();
        const auto base = color->operands().value(0);
        if (components[0] || components[1] || components[2] || !components[3]) {
            return fixedColor();
        }

        auto baseExpression = toColorExpression(base);
        const auto baseValue = resolveColor(base);
        if (!baseExpression || !baseValue || baseValue->alpha() == 0) {
            return fixedColor();
        }

        const auto factor = (components[3].value() / 255.0) / baseValue->alphaF();
        if (std::abs(factor - 1.0) < 0.001) {
            return baseExpression;
        }
        return u"alpha(%1, %2)"_s.arg(baseExpression.value(), formatNumber(factor));
    }
    case Union::ColorData::Type::Empty:
    case Union::ColorData::Type::RGBA:
    case Union::ColorData::Type::AddOperation:
    case Union::ColorData::Type::SubtractOperation:
    case Union::ColorData::Type::MultiplyOperation:
        break;
    }

    return fixedColor();
}
}
