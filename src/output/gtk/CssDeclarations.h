// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#pragma once

#include <optional>

#include <QList>
#include <QString>

namespace Union
{
namespace Gtk
{

struct Declaration {
    QString property;
    QString value;
};

class DeclarationList
{
public:
    void set(const QString &property, const QString &value);
    void remove(const QString &property);
    bool contains(const QString &property) const;
    std::optional<QString> value(const QString &property) const;

    void merge(const DeclarationList &other);

    bool isEmpty() const;
    QList<Declaration> entries() const;

    // What's different from reference. Anything only in reference gets reset
    // to a neutral value
    DeclarationList difference(const DeclarationList &reference) const;

    QString toString() const;

private:
    QList<Declaration> m_entries;
};

QString toPx(qreal value);
QString shorthand(const QString &top, const QString &right, const QString &bottom, const QString &left);
}
}
