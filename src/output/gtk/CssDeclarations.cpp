// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#include "CssDeclarations.h"

#include <algorithm>
#include <cmath>

#include <QHash>

using namespace Qt::StringLiterals;

namespace Union::Gtk
{

static QString neutralValue(const QString &property)
{
    static const QHash<QString, QString> values = {
        {u"background-color"_s, u"transparent"_s},
        {u"background-image"_s, u"none"_s},
        {u"border-width"_s, u"0"_s},
        {u"border-style"_s, u"none"_s},
        {u"border-color"_s, u"transparent"_s},
        {u"border-radius"_s, u"0"_s},
        {u"box-shadow"_s, u"none"_s},
        {u"outline-width"_s, u"0"_s},
        {u"outline-style"_s, u"none"_s},
        {u"outline-color"_s, u"transparent"_s},
        {u"outline-offset"_s, u"0"_s},
        {u"-gtk-outline-radius"_s, u"0"_s},
        {u"padding"_s, u"0"_s},
        {u"margin"_s, u"0"_s},
        {u"min-width"_s, u"0"_s},
        {u"min-height"_s, u"0"_s},
        {u"opacity"_s, u"1"_s},
        {u"-gtk-icon-source"_s, u"none"_s},
        {u"border-spacing"_s, u"0"_s},
        {u"text-decoration-line"_s, u"none"_s},
        {u"letter-spacing"_s, u"0"_s},
        {u"text-transform"_s, u"none"_s},
    };

    return values.value(property, u"inherit"_s);
}

void DeclarationList::set(const QString &property, const QString &value)
{
    for (auto &entry : m_entries) {
        if (entry.property == property) {
            entry.value = value;
            return;
        }
    }
    m_entries.append(Declaration{property, value});
}

void DeclarationList::remove(const QString &property)
{
    m_entries.removeIf([&property](const Declaration &entry) {
        return entry.property == property;
    });
}

bool DeclarationList::contains(const QString &property) const
{
    return std::any_of(m_entries.constBegin(), m_entries.constEnd(), [&property](const Declaration &entry) {
        return entry.property == property;
    });
}

std::optional<QString> DeclarationList::value(const QString &property) const
{
    for (const auto &entry : m_entries) {
        if (entry.property == property) {
            return entry.value;
        }
    }
    return std::nullopt;
}

void DeclarationList::merge(const DeclarationList &other)
{
    for (const auto &entry : other.m_entries) {
        set(entry.property, entry.value);
    }
}

bool DeclarationList::isEmpty() const
{
    return m_entries.isEmpty();
}

QList<Declaration> DeclarationList::entries() const
{
    return m_entries;
}

DeclarationList DeclarationList::difference(const DeclarationList &reference) const
{
    DeclarationList result;

    for (const auto &entry : m_entries) {
        if (reference.value(entry.property) != entry.value) {
            result.set(entry.property, entry.value);
        }
    }

    for (const auto &entry : reference.m_entries) {
        if (!contains(entry.property)) {
            const auto neutral = neutralValue(entry.property);
            if (neutral != entry.value) {
                result.set(entry.property, neutral);
            }
        }
    }

    return result;
}

QString DeclarationList::toString() const
{
    QString result;
    for (const auto &entry : m_entries) {
        result += u"    "_s + entry.property + u": "_s + entry.value + u";\n"_s;
    }
    return result;
}

QString toPx(qreal value)
{
    if (std::abs(value) < 0.005) {
        return u"0"_s;
    }
    const auto rounded = std::round(value * 100.0) / 100.0;
    return QString::number(rounded, 'g', 8) + u"px"_s;
}

QString shorthand(const QString &top, const QString &right, const QString &bottom, const QString &left)
{
    if (left == right) {
        if (top == bottom) {
            if (top == left) {
                return top;
            }
            return top + u' ' + right;
        }
        return top + u' ' + right + u' ' + bottom;
    }
    return top + u' ' + right + u' ' + bottom + u' ' + left;
}
}
