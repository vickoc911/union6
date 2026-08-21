// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "FrameElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

#include "SharedNames.h"

using namespace Qt::StringLiterals;

FrameElement::FrameElement(const QStyleOptionFrame *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_frameOption(option)
{
    update();
}

FrameElement::~FrameElement()
{
}

void FrameElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    updateSubElementList();
    layout();
}

void FrameElement::drawBackground(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_frameOption->rect, m_backgroundProperties);
}

void FrameElement::drawFrame(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_frameOption->rect, m_backgroundProperties, BackgroundParts::FrameOnly);
}

void FrameElement::updateSubElementList()
{
    m_subElementList.clear();
    m_subElementList.append(ElementString::Frame);
}
