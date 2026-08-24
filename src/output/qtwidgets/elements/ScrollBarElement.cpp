// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ScrollBarElement.h"
#include "SharedNames.h"
#include "StyleUtils.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

ScrollBarElement::ScrollBarElement(const QStyleOptionSlider *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_scrollBarOption(option)
    , m_horizontal(false)
{
    update();
}

ScrollBarElement::~ScrollBarElement()
{
}

void ScrollBarElement::update()
{
    m_horizontal = (m_scrollBarOption->state.testFlag(QStyle::State_Horizontal));
    updateSubElementList();
    layout();
}

void ScrollBarElement::layout()
{
    // Background and content is separate
    m_backgroundElementList = prepareElements(m_scrollBarOption, m_widget);
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
        m_layoutMap = layoutMap(m_backgroundElementList, m_scrollBarOption, m_subElementList);
    }

    m_indicatorElementList = prepareElements(m_scrollBarOption, m_widget, {ElementString::Handle});
    if (!m_indicatorElementList.empty()) {
        m_indicatorProperties = queryProperties(m_indicatorElementList);
    }

    m_contentElementList = prepareElements(m_scrollBarOption, m_widget, m_subElementList);
    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
        m_isValid = true;
    } else {
        m_isValid = false;
        qWarning() << "Could not find elementlist for this element!";
    }
}

void ScrollBarElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }

    drawBackground(painter);
    drawIndicator(painter);
}

void ScrollBarElement::drawBackground(QPainter *painter) const
{
    const auto rect = subControlRect(QStyle::SC_ScrollBarGroove);
    drawBackgroundRectangle(painter, rect, m_backgroundProperties);
}

void ScrollBarElement::drawIndicator(QPainter *painter) const
{
    if (m_scrollBarOption->subControls & QStyle::SC_ScrollBarSlider) {
        QStyleOptionSlider subopt = *m_scrollBarOption;
        subopt.rect = m_scrollBarOption->rect;
        subopt.state = m_scrollBarOption->state;
        subopt.rect = subControlRect(QStyle::SC_ScrollBarSlider);
        if (subopt.rect.isValid()) {
            if (!(m_scrollBarOption->activeSubControls & QStyle::SC_ScrollBarSlider)) {
                subopt.state &= ~(QStyle::State_Sunken | QStyle::State_MouseOver);
            }
            drawBackgroundRectangle(painter, subopt.rect, m_indicatorProperties);

            if (m_scrollBarOption->state & QStyle::State_HasFocus) {
                m_style->drawPrimitive(QStyle::PE_FrameFocusRect, &subopt, painter);
            }
        }
    }
}

void ScrollBarElement::updateSubElementList()
{
    m_subElementList.clear();
    m_subElementList.append(ElementString::Handle);
}

QSize ScrollBarElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    return contentsSizeFromStyle;
}

QRect ScrollBarElement::subControlRect(QStyle::SubControl subControl) const
{
    if (!m_isValid) {
        qWarning() << "subControlRect for " << subControl << "is not valid";
        return QRect();
    }

    // Copied from Breeze
    auto rect = m_scrollBarOption->rect;
    if (m_widget) {
        rect = m_widget->visibleRegion().boundingRect();
    }
    if (subControl == QStyle::SC_ScrollBarSlider) {
        auto groove = m_style->visualRect(m_scrollBarOption->direction, rect, subControlRect(QStyle::SC_ScrollBarGroove));

        int space(m_horizontal ? groove.width() : groove.height());
        int thickness = 0;
        QMargins padding;

        if (m_backgroundProperties->layout() && m_backgroundProperties->layout()->padding()) {
            padding = m_backgroundProperties->layout()->padding()->toMargins().toMargins();
            if (m_scrollBarOption->orientation == Qt::Horizontal) {
                thickness = m_backgroundProperties->layout()->height().value_or(0);
            } else {
                thickness = m_backgroundProperties->layout()->width().value_or(0);
            }
        }

        // Return early with just padding changes
        if (m_scrollBarOption->minimum == m_scrollBarOption->maximum) {
            const auto rect = QRect(groove.left(), groove.top(), groove.width(), groove.height());
            if (m_horizontal) {
                return m_style->visualRect(m_scrollBarOption->direction, rect, rect.marginsRemoved(padding));
            } else {
                return m_style->visualRect(m_scrollBarOption->direction, rect, rect.marginsRemoved(padding));
            }
        }

        int sliderSize = space * qreal(m_scrollBarOption->pageStep) / (m_scrollBarOption->maximum - m_scrollBarOption->minimum + m_scrollBarOption->pageStep);
        sliderSize = qMax(sliderSize, qMax(thickness, m_style->pixelMetric(QStyle::PM_ScrollBarSliderMin, m_scrollBarOption, m_widget)));
        sliderSize = qMin(sliderSize, space);
        space -= sliderSize;
        if (space <= 0) {
            const auto rect = QRect(groove.left(), groove.top(), groove.width(), groove.height());
            if (m_horizontal) {
                return m_style->visualRect(m_scrollBarOption->direction, rect, rect.marginsRemoved(padding));
            } else {
                return m_style->visualRect(m_scrollBarOption->direction, rect, rect.marginsRemoved(padding));
            }
        }
        int pos =
            qRound(qreal(m_scrollBarOption->sliderPosition - m_scrollBarOption->minimum) / (m_scrollBarOption->maximum - m_scrollBarOption->minimum) * space);
        if (m_scrollBarOption->upsideDown) {
            pos = space - pos;
        }
        if (m_horizontal) {
            const auto rect = QRect(groove.left() + pos, groove.top(), sliderSize, groove.height());
            return m_style->visualRect(m_scrollBarOption->direction, rect, rect.marginsRemoved(padding));
        } else {
            const auto rect = QRect(groove.left(), groove.top() + pos, groove.width(), sliderSize);
            return m_style->visualRect(m_scrollBarOption->direction, rect, rect.marginsRemoved(padding));
        }
    } else if (subControl == QStyle::SC_ScrollBarGroove) {
        return m_style->visualRect(m_scrollBarOption->direction, m_scrollBarOption->rect, rect);
    } else {
        return QRect();
    }
}
