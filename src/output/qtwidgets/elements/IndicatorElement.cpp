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
    drawElement(painter, u"arrow-left-symbolic"_s, {ElementString::IndicatorArrowLeft});
}

void IndicatorElement::drawArrowRight(QPainter *painter) const
{
    drawElement(painter, u"arrow-right-symbolic"_s, {ElementString::IndicatorArrowRight});
}

void IndicatorElement::drawArrowDown(QPainter *painter) const
{
    drawElement(painter, u"arrow-down-symbolic"_s, {ElementString::IndicatorArrowDown});
}

void IndicatorElement::drawArrowUp(QPainter *painter) const
{
    drawElement(painter, u"arrow-up-symbolic"_s, {ElementString::IndicatorArrowUp});
}

void IndicatorElement::drawDropDown(QPainter *painter) const
{
    drawElement(painter, u"arrow-down-symbolic"_s, {ElementString::IndicatorButtonDropDown});
}

void IndicatorElement::drawElement(QPainter *painter, const QString &defaultIconName, QStringList targetHierarchy) const
{
    auto elements = prepareElements(m_indicatorOption, m_widget, targetHierarchy);
    auto properties = queryProperties(elements);
    auto icon = m_style->unionIcon(properties, defaultIconName);
    drawIconAtRect(painter, icon, m_indicatorOption->rect);
}

qreal IndicatorElement::listViewIconSize() const
{
    return querySize({ElementString::ListViewIconSize}).width();
}
qreal IndicatorElement::smallIconSize() const
{
    return querySize({ElementString::SmallIconSize}).width();
}
qreal IndicatorElement::iconViewIconSize() const
{
    return querySize({ElementString::IconViewIconSize}).width();
}
qreal IndicatorElement::largeIconSize() const
{
    return querySize({ElementString::LargeIconSize}).width();
}
qreal IndicatorElement::messageBoxIconSize() const
{
    return querySize({ElementString::MessageBoxIconSize}).width();
}
qreal IndicatorElement::textCursorWidth() const
{
    return querySize({ElementString::TextCursorWidth}).width();
}
