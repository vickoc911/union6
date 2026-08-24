// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "MenuElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

#include "SharedNames.h"

using namespace Qt::StringLiterals;

MenuElement::MenuElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_menuOption(option)
{
    update();
}

MenuElement::~MenuElement()
{
}

void MenuElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    updateSubElementList();
    layout();
}

void MenuElement::drawBackground(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_menuOption->rect, m_backgroundProperties);
}

void MenuElement::drawFrame(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_menuOption->rect, m_backgroundProperties, BackgroundParts::FrameOnly);
}

void MenuElement::updateSubElementList()
{
    m_subElementList.clear();
    m_subElementList.append(ElementString::Frame);
}

QSizeF MenuElement::contentsSize(const QSizeF &contentsSizeFromStyle) const
{
    return applyPaddingToSize(contentsSizeFromStyle);
}
