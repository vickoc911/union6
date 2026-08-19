// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "GroupBoxElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QGroupBox>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

GroupBoxElement::GroupBoxElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_groupBoxOption(qstyleoption_cast<const QStyleOptionGroupBox *>(option))
    , m_isCheckable(false)
{
    if (auto groupBox = qobject_cast<const QGroupBox *>(m_widget)) {
        if (groupBox->isCheckable()) {
            m_isCheckable = true;
        }
    }
    if (m_groupBoxOption) {
        if (!m_groupBoxOption->text.isEmpty()) {
            setText(m_groupBoxOption->text);
        }
    }
    layout();
}

GroupBoxElement::~GroupBoxElement()
{
}

QSize GroupBoxElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    return applyPaddingToSize(contentsSizeFromStyle);
}

void GroupBoxElement::layout()
{
    // We only layout by background, m_contentElementList etc are ignored
    if (m_backgroundElementList.isEmpty()) {
        m_backgroundElementList = prepareElements(m_styleOption, m_widget);
    }
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
        m_isValid = true;
    }
}

QRect GroupBoxElement::subControlRect(QStyle::SubControl subControl) const
{
    if (!m_isValid) {
        qWarning() << "subControlRect for " << subControl << "is not valid";
        return QRect();
    }

    QRect finalRect;

    switch (subControl) {
    case QStyle::SC_GroupBoxLabel: {
        auto map = layoutMap(m_backgroundElementList, m_styleOption, {ElementString::Text});
        finalRect = map[ElementString::Text].rect.toRect();
    } break;
    case QStyle::SC_GroupBoxContents: {
        auto map = layoutMap(m_backgroundElementList, m_styleOption, {ElementString::GroupBox, ElementString::Text});
        auto textRect = map[ElementString::Text].rect.toRect();
        QMargins padding;
        QRect frameRect = m_groupBoxOption->rect;
        frameRect = frameRect.adjusted(0, textRect.height(), 0, 0);
        if (m_backgroundProperties->layout()) {
            padding = m_backgroundProperties->layout()->padding()->toMargins().toMargins();
        }
        frameRect.adjust(0, 0, 0, -padding.bottom());
        const int fontMetricsHeight = hasText() ? 0 : m_groupBoxOption->fontMetrics.height();
        int indicatorHeight = 0;
        if (m_isCheckable) {
            indicatorHeight = m_style->pixelMetric(QStyle::PM_IndicatorHeight, m_styleOption, m_widget);
        }
        const int topMargin = qMax(indicatorHeight, fontMetricsHeight);
        return frameRect.adjusted(padding.left(), padding.top() + topMargin, -padding.bottom(), -padding.right());
    } break;
    case QStyle::SC_GroupBoxCheckBox: {
        auto map = layoutMap(m_backgroundElementList, m_styleOption, {ElementString::Icon});
        finalRect = map[ElementString::Icon].rect.toRect();
    } break;
    case QStyle::SC_GroupBoxFrame: {
        return m_styleOption->rect;
    }
    default:
        break;
    }

    return m_style->visualRect(m_styleOption->direction, m_styleOption->rect, finalRect);
}

void GroupBoxElement::drawText(QPainter *painter) const
{
    if ((m_groupBoxOption->subControls & QStyle::SC_GroupBoxLabel) && hasText()) {
        QRect textRect = subControlRect(QStyle::SC_GroupBoxLabel);
        int textFlags = Qt::AlignLeading | Qt::AlignVCenter;
        const bool enabled = m_styleOption->state.testFlag(QStyle::State_Enabled);
        QColor penColor = m_styleOption->palette.text().color();
        if (m_backgroundProperties->text()) {
            auto textColor = m_backgroundProperties->text()->color();
            if (textColor) {
                penColor = textColor->toQColor();
            }
            textFlags = textFlagsFromProperties(m_backgroundProperties, false);
        }
        painter->save();
        if (m_backgroundProperties->text() && m_backgroundProperties->text()->font().has_value()) {
            painter->setFont(m_backgroundProperties->text()->font().value());
        }
        painter->setPen(penColor);
        m_style->drawItemText(painter, textRect, textFlags, m_styleOption->palette, enabled, m_text);
        painter->restore();
    }
}

void GroupBoxElement::drawIcon(QPainter *painter) const
{
    if (m_isCheckable) {
        QStyleOptionButton checkbox;
        checkbox.rect = subControlRect(QStyle::SC_GroupBoxCheckBox);
        checkbox.state = m_groupBoxOption->state;
        m_style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkbox, painter, m_widget);
    }
}
