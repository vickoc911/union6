// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ToolButtonElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

ToolButtonElement::ToolButtonElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_toolButtonOption(qstyleoption_cast<const QStyleOptionToolButton *>(option))
{
    if (m_toolButtonOption) {
        m_indicatorElementList = prepareElements(m_toolButtonOption, m_widget, {u"Indicator"_s});
        if (!m_indicatorElementList.isEmpty()) {
            m_indicatorProperties = queryProperties(m_indicatorElementList);
            if (m_indicatorProperties->icon()) {
                setIndicator(QIcon::fromTheme(m_indicatorProperties->icon()->name().value_or(QString())));
            }
        }

        if (!m_toolButtonOption->icon.isNull()) {
            setIcon(m_toolButtonOption->icon);
        }
        if (!m_toolButtonOption->text.isEmpty()) {
            setText(m_toolButtonOption->text);
        }
    }
    updateSubElementList();
    layout();
}

ToolButtonElement::~ToolButtonElement()
{
}

void ToolButtonElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }

    drawBackground(painter, m_styleOption->rect, m_backgroundProperties);
    drawIcon(painter);
    drawText(painter);
    drawIndicator(painter);
}

QSize ToolButtonElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    Q_UNUSED(contentsSizeFromStyle);
    QSize size = subControlRect(QStyle::SC_ToolButton).size();
    size = applyPaddingToSize(size);

    if (m_toolButtonOption && m_indicatorProperties && m_indicatorProperties->layout()) {
        if (m_toolButtonOption->toolButtonStyle != Qt::ToolButtonTextUnderIcon) {
            size.rwidth() += m_indicatorProperties->layout()->width().value_or(0);
        } else {
            size.rheight() += m_indicatorProperties->layout()->height().value_or(0);
        }
    }

    return size;
}

QRect ToolButtonElement::subControlRect(QStyle::SubControl subControl) const
{
    if (!m_isValid) {
        qWarning() << "subControlRect for " << subControl << "is not valid";
        return QRect();
    }

    QRect backgroundRect = backgroundRectangle(m_styleOption, m_backgroundProperties).toRect();
    if (subControl == QStyle::SC_ToolButton) {
        return backgroundRect;
    }
    if (subControl == QStyle::SC_ToolButtonMenu) {
        QRect menuRect = m_layoutMap[u"Indicator"_s].rect.toRect();
        // Set the click area to full height/width, so that its easier to click
        if (m_toolButtonOption->toolButtonStyle != Qt::ToolButtonTextUnderIcon) {
            menuRect.setTop(backgroundRect.top());
            menuRect.setBottom(backgroundRect.bottom());
        } else {
            menuRect.setLeft(backgroundRect.left());
            menuRect.setRight(backgroundRect.right());
        }
        return menuRect;
    }

    QRect rect = m_styleOption->rect;
    QRect unifiedRect;
    for (const auto &m : m_layoutMap) {
        unifiedRect = unifiedRect.united(m.rect.toRect());
    }
    rect = unifiedRect;
    return rect;
}

ToolButtonElement::Ptr ToolButtonElement::create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
{
    return std::make_shared<ToolButtonElement>(option, style, widget);
}

void ToolButtonElement::drawText(QPainter *painter) const
{
    if (m_toolButtonOption->toolButtonStyle == Qt::ToolButtonIconOnly) {
        return;
    }
    if (hasText()) {
        QRect textRect = m_layoutMap[u"Text"_s].rect.toRect();
        int textFlags = Qt::AlignLeading | Qt::AlignVCenter;
        const bool enabled = m_styleOption->state.testFlag(QStyle::State_Enabled);
        QColor penColor = m_styleOption->palette.text().color();
        // TODO: hide mnemonics if requested
        if (m_contentProperties->text()) {
            auto textColor = m_contentProperties->text()->color();
            if (textColor) {
                penColor = textColor->toQColor();
            }
            textFlags = textFlagsFromProperties(m_contentProperties, true);
        }
        painter->save();
        painter->setPen(penColor);
        m_style->drawItemText(painter, textRect, textFlags, m_styleOption->palette, enabled, m_text);
        painter->restore();
    }
}

void ToolButtonElement::drawIcon(QPainter *painter) const
{
    if (m_toolButtonOption->toolButtonStyle == Qt::ToolButtonTextOnly) {
        return;
    }

    QRect iconRect = m_layoutMap[u"Icon"_s].rect.toRect();
    if (m_toolButtonOption->features.testFlag(QStyleOptionToolButton::Arrow)) {
        auto subopt = *m_toolButtonOption;
        subopt.rect = iconRect;
        switch (m_toolButtonOption->arrowType) {
        case Qt::LeftArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowLeft, &subopt, painter, m_widget);
            break;
        case Qt::RightArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &subopt, painter, m_widget);
            break;
        case Qt::UpArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowUp, &subopt, painter, m_widget);
            break;
        case Qt::DownArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &subopt, painter, m_widget);
            break;
        default:
            break;
        }
        return;
    } else if (hasIcon()) {
        const bool enabled = m_styleOption->state.testFlag(QStyle::State_Enabled);
        const QPalette activePalette = m_styleOption->palette;
        const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
        auto iconSize = iconRect.size();

        // Toolbutton can override the regular icon size
        if (const auto *toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton *>(m_styleOption)) {
            // However avoid resizing any icon (like indicators) inside toolbutton, only the main icon
            if (toolButtonOption->icon.name() == m_icon.name()) {
                iconSize = toolButtonOption->iconSize;
            }
        }
        const QPixmap pixmap = m_icon.pixmap(iconSize, dpr, enabled ? QIcon::Normal : QIcon::Disabled);

        QColor penColor = m_styleOption->palette.text().color(); // Use text color as fallback
        if (m_contentProperties->icon() && m_contentProperties->icon()->color().has_value()) {
            auto iconColor = m_contentProperties->icon()->color();
            penColor = iconColor->toQColor();
        }
        painter->save();
        painter->setPen(penColor);
        m_style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
        painter->restore();
    }
}
