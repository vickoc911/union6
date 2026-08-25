// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "MenuBarElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

MenuBarElement::MenuBarElement(const QStyleOptionMenuItem *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_menuItemOption(option)
{
    update();
}

MenuBarElement::~MenuBarElement()
{
}

void MenuBarElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    layout();
}

QVariantMap MenuBarElement::elementAttributes() const
{
    return QVariantMap();
}

QStringList MenuBarElement::elementHints() const
{
    QStringList hints;
    if (m_menuItemOption->checked) {
        hints.append(u"with-submenu"_s);
    }
    if (m_menuItemOption->menuItemType == QStyleOptionMenuItem::Separator && !m_menuItemOption->text.isEmpty()) {
        hints.append(u"with-title"_s);
    }
    return hints;
}
