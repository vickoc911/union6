// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "SpinBoxElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

SpinBoxElement::SpinBoxElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_spinBoxOption(qstyleoption_cast<const QStyleOptionSpinBox *>(option))
{
    if (m_spinBoxOption) {
        m_indicatorElementList = prepareElements(m_spinBoxOption, m_widget, {u"Indicator"_s});
        if (!m_indicatorElementList.isEmpty()) {
            m_indicatorProperties = queryProperties(m_indicatorElementList);
        }
    }
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

    drawBg(painter);
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
    if (m_spinBoxOption) {
        m_subElementList.append(u"Indicator"_s);
    }
}

QSize SpinBoxElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    QRegion r;
    const auto upRect = subControlRect(QStyle::SC_SpinBoxUp);
    const auto downRect = subControlRect(QStyle::SC_SpinBoxDown);
    const auto editFieldRect = subControlRect(QStyle::SC_SpinBoxEditField);
    const auto frameRect = subControlRect(QStyle::SC_SpinBoxFrame);
    r.setRects({upRect, downRect, editFieldRect, frameRect});
    return r.boundingRect().size().expandedTo(contentsSizeFromStyle);
}

QRect SpinBoxElement::subControlRect(QStyle::SubControl subControl) const
{
    if (!m_isValid) {
        qWarning() << "subControlRect for " << subControl << "is not valid";
        return QRect();
    }

    QRect rect;
    // Based on QCommonStyle. We only draw the "constrained" look for now.
    if (m_spinBoxOption) {
        const QRect buttonRect = m_layoutMap[u"Indicator"_s].rect.toRect();
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
    }
    return rect;
}

SpinBoxElement::Ptr SpinBoxElement::create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
{
    return std::make_shared<SpinBoxElement>(option, style, widget);
}
