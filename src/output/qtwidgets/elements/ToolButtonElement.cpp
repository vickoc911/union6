// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ToolButtonElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

ToolButtonElement::ToolButtonElement(const QStyleOptionToolButton *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_toolButtonOption(option)
    , m_hasIndicator(false)
    , m_hasArrows(false)
    , m_hasIcon(false)
    , m_hasText(false)
{
    update();
}

ToolButtonElement::~ToolButtonElement()
{
}

void ToolButtonElement::update()
{
    m_hasIndicator =
        m_toolButtonOption->features.testFlag(QStyleOptionToolButton::HasMenu) || m_toolButtonOption->features.testFlag(QStyleOptionToolButton::Menu);
    m_hasArrows = m_toolButtonOption->features.testFlag(QStyleOptionToolButton::Arrow) && m_toolButtonOption->toolButtonStyle != Qt::ToolButtonTextOnly;
    m_hasIcon = !m_toolButtonOption->icon.isNull() && m_toolButtonOption->toolButtonStyle != Qt::ToolButtonTextOnly;
    m_hasText = !m_toolButtonOption->text.isEmpty() && m_toolButtonOption->toolButtonStyle != Qt::ToolButtonIconOnly;
    m_indicatorElementList = prepareElements(m_toolButtonOption, m_widget, {ElementString::Indicator});
    setIndicator(QIcon());
    if (!m_indicatorElementList.isEmpty()) {
        m_indicatorProperties = queryProperties(m_indicatorElementList);
        if (m_indicatorProperties->icon()) {
            setIndicator(QIcon::fromTheme(m_indicatorProperties->icon()->name().value_or(QString())));
        }
    }
    setIcon(m_toolButtonOption->icon);
    setText(m_toolButtonOption->text);
    updateSubElementList();
    layout();
}

void ToolButtonElement::updateSubElementList()
{
    m_subElementList.clear();
    if (m_hasIcon || m_hasArrows) {
        m_subElementList.append(ElementString::Icon);
    }
    if (m_hasText) {
        m_subElementList.append(ElementString::Text);
    }
    if (m_hasIndicator) {
        m_subElementList.append(ElementString::Indicator);
    }
}

QSize ToolButtonElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    QSize size = subControlRect(QStyle::SC_ToolButton).size().boundedTo(contentsSizeFromStyle);
    size = applyPaddingToSize(size);

    if (m_indicatorProperties && m_indicatorProperties->layout()) {
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

    QRect backgroundRect = backgroundRectangle(m_toolButtonOption, m_backgroundProperties).toRect();
    if (subControl == QStyle::SC_ToolButton) {
        QRect rect = m_toolButtonOption->rect;
        QRect unifiedRect;
        for (const auto &m : m_layoutMap) {
            unifiedRect = unifiedRect.united(m.rect.toRect());
        }
        rect = unifiedRect;
        return rect;
    }
    if (subControl == QStyle::SC_ToolButtonMenu) {
        QRect menuRect = m_layoutMap[ElementString::Indicator].rect.toRect();
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
    return QRect();
}

void ToolButtonElement::drawText(QPainter *painter) const
{
    if (m_toolButtonOption->toolButtonStyle == Qt::ToolButtonIconOnly) {
        return;
    }
    AbstractElement::drawText(painter);
}

void ToolButtonElement::drawIcon(QPainter *painter) const
{
    if (m_toolButtonOption->toolButtonStyle == Qt::ToolButtonTextOnly) {
        return;
    }

    QRect iconRect = m_layoutMap[ElementString::Icon].rect.toRect();
    if (m_toolButtonOption->features.testFlag(QStyleOptionToolButton::Arrow)) {
        auto subopt = *m_toolButtonOption;
        subopt.rect = iconRect;
        switch (m_toolButtonOption->arrowType) {
        case Qt::LeftArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowLeft, &subopt, painter);
            break;
        case Qt::RightArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &subopt, painter);
            break;
        case Qt::UpArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowUp, &subopt, painter);
            break;
        case Qt::DownArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &subopt, painter);
            break;
        default:
            break;
        }
        return;
    } else if (hasIcon()) {
        const bool enabled = m_toolButtonOption->state.testFlag(QStyle::State_Enabled);
        const QPalette activePalette = m_toolButtonOption->palette;
        const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
        auto iconSize = iconRect.size();

        // Toolbutton can override the regular icon size
        if (const auto *toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton *>(m_toolButtonOption)) {
            // However avoid resizing any icon (like indicators) inside toolbutton, only the main icon
            if (toolButtonOption->icon.name() == m_icon.name()) {
                iconSize = toolButtonOption->iconSize;
            }
        }
        const QPixmap pixmap = m_icon.pixmap(iconSize, dpr, enabled ? QIcon::Normal : QIcon::Disabled);

        QColor penColor = m_toolButtonOption->palette.text().color(); // Use text color as fallback
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
