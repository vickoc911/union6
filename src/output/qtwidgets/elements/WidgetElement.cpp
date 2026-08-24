// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "WidgetElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

WidgetElement::WidgetElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_widgetOption(option)
{
    update();
}

WidgetElement::~WidgetElement()
{
}

void WidgetElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    updateSubElementList();
    layout();
}

void WidgetElement::drawBackground(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_widgetOption->rect, m_backgroundProperties);
}

void WidgetElement::drawFrame(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_widgetOption->rect, m_backgroundProperties, BackgroundParts::FrameOnly);
}
