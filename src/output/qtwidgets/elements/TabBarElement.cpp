// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "TabBarElement.h"
#include "BackgroundDrawing.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

TabBarElement::TabBarElement(const QStyleOptionTabBarBase *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_tabBarOption(option)
{
    setIcon(QIcon());
    update();
}

TabBarElement::~TabBarElement()
{
}

void TabBarElement::update()
{
    layout();
}

void TabBarElement::drawBackground(QPainter *painter) const
{
    drawBackgroundRectangle(painter,
                            m_tabBarOption->rect,
                            m_backgroundProperties,
                            m_tabBarOption->documentMode ? BackgroundParts::PanelOnly : BackgroundParts::All);
}

qreal TabBarElement::scrollButtonWidth() const
{
    return querySize({ElementString::TabScrollButton}).width();
}
