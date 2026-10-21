// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#include <QGuiApplication>
#include <QStandardPaths>

#include "ThemeGenerator.h"
#include <StyleRegistry.h>

#include "gtk_logging.h"

using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    Union::StyleRegistry::instance()->load();

    const auto style = Union::StyleRegistry::instance()->defaultStyle();
    if (!style) {
        qCWarning(UNION_GTK) << "Could not load the Union style";
        return 1;
    }

    const auto directory = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + u"/themes/union"_s;
    Union::Gtk::ThemeGenerator(style).writeTheme(directory);

    return 0;
}
