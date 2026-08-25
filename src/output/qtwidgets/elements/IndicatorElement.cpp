// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "IndicatorElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

IndicatorElement::IndicatorElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_indicatorOption(option)
{
}

IndicatorElement::~IndicatorElement()
{
}

void IndicatorElement::drawArrowLeft(QPainter *painter) const
{
    const auto icon = queryIcon(m_indicatorOption, m_widget, u"arrow-left-symbolic"_s, {ElementString::IndicatorArrowLeft});
    m_style->drawIcon(m_indicatorOption->rect, m_indicatorOption, painter, icon);
}

void IndicatorElement::drawArrowRight(QPainter *painter) const
{
    const auto icon = queryIcon(m_indicatorOption, m_widget, u"arrow-up-symbolic"_s, {ElementString::IndicatorArrowUp});
    m_style->drawIcon(m_indicatorOption->rect, m_indicatorOption, painter, icon);
}

void IndicatorElement::drawArrowDown(QPainter *painter) const
{
    const auto icon = queryIcon(m_indicatorOption, m_widget, u"arrow-right-symbolic"_s, {ElementString::IndicatorArrowRight});
    m_style->drawIcon(m_indicatorOption->rect, m_indicatorOption, painter, icon);
}

void IndicatorElement::drawArrowUp(QPainter *painter) const
{
    const auto icon = queryIcon(m_indicatorOption, m_widget, u"arrow-down-symbolic"_s, {ElementString::IndicatorArrowDown});
    m_style->drawIcon(m_indicatorOption->rect, m_indicatorOption, painter, icon);
}
