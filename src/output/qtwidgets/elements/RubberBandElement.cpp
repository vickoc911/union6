// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "RubberBandElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

RubberBandElement::RubberBandElement(const QStyleOptionRubberBand *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_rubberBandOption(option)
{
    update();
}

RubberBandElement::~RubberBandElement()
{
}

void RubberBandElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    layout();
}

void RubberBandElement::layout()
{
    // Background and content is separate
    m_backgroundElementList = prepareElements(m_styleOption, m_widget, {ElementString::RubberBand});
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
        m_isValid = true;
    }
}

void RubberBandElement::drawBackground(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_rubberBandOption->rect, m_backgroundProperties);
}
