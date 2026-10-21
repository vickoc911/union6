// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#pragma once

#include <map>
#include <memory>

#include <QHash>
#include <QString>

#include "ElementMap.h"

namespace Union
{
class Style;

namespace Gtk
{
class ThemeAssets;

struct GeneratedStylesheet {
    QString css;
    std::map<QString, QByteArray> assets;
};

class ThemeGenerator : private RuleContext
{
public:
    explicit ThemeGenerator(std::shared_ptr<Union::Style> style);

    bool writeTheme(const QString &themeDirectory) const;

private:
    std::shared_ptr<Union::Properties::StylePropertyGroup> query(const ElementPath &path, Union::Element::States states) const override;

    std::shared_ptr<Union::Properties::StylePropertyGroup> queryVariant(const ElementPath &path, int anchor, const StateVariant &variant) const;
    QString palette() const;
    QHash<QString, QString> metrics(ThemeAssets &assets) const;
    QString rules(GtkVersion version, ThemeAssets &assets) const;
    GeneratedStylesheet generate(GtkVersion version) const;

    std::shared_ptr<Union::Style> m_style;
};
}
}
