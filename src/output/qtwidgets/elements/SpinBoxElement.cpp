// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "SpinBoxElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

SpinBoxElement::SpinBoxElement(const QStyleOptionSpinBox *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_spinBoxOption(option)
    , m_hasButtons(true)
{
    m_indicatorElementList = prepareElements(m_spinBoxOption, m_widget, {ElementString::Indicator});
    if (!m_indicatorElementList.isEmpty()) {
        m_indicatorProperties = queryProperties(m_indicatorElementList);
    }
    m_hasButtons = (m_spinBoxOption->buttonSymbols != QAbstractSpinBox::NoButtons);
    updateSubElementList();
    layout();
}

SpinBoxElement::~SpinBoxElement()
{
}

void SpinBoxElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }

    drawBackground(painter);
    // For spinbox we need to manually create the indicator buttons
    if (m_spinBoxOption->buttonSymbols != QAbstractSpinBox::NoButtons) {
        bool arrows = (m_spinBoxOption->buttonSymbols == QAbstractSpinBox::UpDownArrows);
        // Increase
        auto up = *m_spinBoxOption;
        up.rect = subControlRect(QStyle::SC_SpinBoxUp);
        m_style->drawPrimitive(arrows ? QStyle::PE_IndicatorSpinUp : QStyle::PE_IndicatorSpinPlus, &up, painter, m_widget);
        // Decrease
        auto down = *m_spinBoxOption;
        down.rect = subControlRect(QStyle::SC_SpinBoxDown);
        m_style->drawPrimitive(arrows ? QStyle::PE_IndicatorSpinDown : QStyle::PE_IndicatorSpinMinus, &down, painter, m_widget);
    }
}

void SpinBoxElement::updateSubElementList()
{
    m_subElementList.clear();
    m_subElementList.append(ElementString::Indicator);
}

QSize SpinBoxElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    const int frameWidth = m_style->pixelMetric(QStyle::PM_SpinBoxFrameWidth, m_spinBoxOption, m_widget);
    auto size = contentsSizeFromStyle;
    size += QSize(2 * frameWidth, 2 * frameWidth);
    if (m_hasButtons) {
        auto topButton = subControlRect(QStyle::SC_SpinBoxUp).size();
        auto bottomButton = subControlRect(QStyle::SC_SpinBoxDown).size();
        const auto height = topButton.height() + bottomButton.height();
        const auto buttonWidth = topButton.expandedTo(bottomButton).width();
        size.rwidth() += buttonWidth;
        size.setHeight(height);
    }

    return size;
}

QRect SpinBoxElement::subControlRect(QStyle::SubControl subControl) const
{
    if (!m_isValid) {
        qWarning() << "subControlRect for " << subControl << "is not valid";
        return QRect();
    }

    QRect rect;
    // Based on QCommonStyle. We only draw the "constrained" look for now.
    const QRect buttonRect = m_layoutMap[ElementString::Indicator].rect.toRect();
    QRect bgRect = m_spinBoxOption->rect;
    if (m_backgroundProperties->layout()) {
        bgRect.setWidth(qMax(bgRect.width(), (int)m_backgroundProperties->layout()->width().value_or(0)));
        bgRect.setHeight(qMax(bgRect.height(), (int)m_backgroundProperties->layout()->height().value_or(0)));
    }
    const bool noButtons = (m_spinBoxOption->buttonSymbols == QAbstractSpinBox::NoButtons);
    const int y = m_spinBoxOption->rect.y();
    const int x = m_spinBoxOption->rect.x() + m_spinBoxOption->rect.width() - buttonRect.width();

    if (subControl == QStyle::SC_SpinBoxUp) {
        rect = noButtons ? QRect() : QRect(x, y, buttonRect.width(), buttonRect.height());
    }
    if (subControl == QStyle::SC_SpinBoxDown) {
        rect = noButtons ? QRect() : QRect(x, y + buttonRect.height(), buttonRect.width(), buttonRect.height());
    }
    if (subControl == QStyle::SC_SpinBoxEditField) {
        if (noButtons) {
            rect = QRect(0, 0, bgRect.width(), bgRect.height());
        } else {
            rect = QRect(0, 0, x, bgRect.height());
        }
    }
    if (subControl == QStyle::SC_SpinBoxFrame) {
        rect = bgRect;
    }
    rect = m_style->visualRect(m_spinBoxOption->direction, m_spinBoxOption->rect, rect);
    return rect;
}
