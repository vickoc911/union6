// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "SplitterElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

#include "SharedNames.h"

using namespace Qt::StringLiterals;

SplitterElement::SplitterElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_splitterOption(option)
{
    update();
}

SplitterElement::~SplitterElement()
{
}

void SplitterElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    updateSubElementList();
    layout();
}

void SplitterElement::layout()
{
    m_backgroundElementList = prepareElements(m_styleOption, m_widget, {ElementString::Splitter});
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
        m_isValid = true;
    }
}

void SplitterElement::drawBackground(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_splitterOption->rect, m_backgroundProperties);
}
