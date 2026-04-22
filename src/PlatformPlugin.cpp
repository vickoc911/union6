// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Arjen Hiemstra <ahiemstra@heimr.nl>

#include "PlatformPlugin.h"
#include <QGuiApplication>
#include <QQuickWindow>

#include <QIcon>

using namespace Union;
using namespace Qt::StringLiterals;

PlatformPlugin::PlatformPlugin(QObject *parent)
    : Plugin(parent)
{
    // NativeTextRendering is still distorted sometimes with fractional scale factors
    // Given Qt disables all hinting with native rendering when any scaling is used anyway
    // we can use Qt's rendering throughout
    // QTBUG-126577
    if (qApp->devicePixelRatio() == 1.0) {
        QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);
    } else {
        QQuickWindow::setTextRenderType(QQuickWindow::QtTextRendering);
    }
}

QString PlatformPlugin::defaultInputPlugin()
{
    return QString{};
}

QIcon PlatformPlugin::platformIcon(const QString &name, [[maybe_unused]] const QColor &color)
{
    return QIcon::fromTheme(name);
}

bool PlatformPlugin::smoothScroll()
{
    return true;
}