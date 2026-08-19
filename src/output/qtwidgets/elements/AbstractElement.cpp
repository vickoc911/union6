// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "AbstractElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

AbstractElement::AbstractElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : QObject(nullptr)
    , m_styleOption(option)
    , m_style(style)
    , m_widget(widget)
    , m_icon(QIcon())
    , m_text(QString())
    , m_indicator(QIcon())
    , m_isValid(false)
{
}

AbstractElement::~AbstractElement()
{
}

AbstractElement::Ptr AbstractElement::create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
{
    return std::make_shared<AbstractElement>(option, style, widget);
}

QIcon AbstractElement::icon() const
{
    return m_icon;
}

void AbstractElement::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

bool AbstractElement::hasIcon() const
{
    return !m_icon.isNull();
}

QString AbstractElement::text() const
{
    return m_text;
}

void AbstractElement::setText(const QString &text)
{
    m_text = text;
}

bool AbstractElement::hasText() const
{
    return !m_text.isEmpty();
}

QIcon AbstractElement::indicator() const
{
    return m_indicator;
}

void AbstractElement::setIndicator(const QIcon &indicator)
{
    m_indicator = indicator;
}

bool AbstractElement::hasIndicator() const
{
    return !m_indicator.isNull();
}

bool AbstractElement::isValid() const
{
    return m_isValid;
}

void AbstractElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }
    drawBg(painter);
    drawIcon(painter);
    drawText(painter);
}

void AbstractElement::layout()
{
    // Background and content is separate
    if (m_backgroundElementList.isEmpty()) {
        m_backgroundElementList = prepareElements(m_styleOption, m_widget);
    }
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
        m_layoutMap = layoutMap(m_backgroundElementList, m_styleOption, m_subElementList);
    }

    if (m_contentElementList.isEmpty()) {
        m_contentElementList = prepareElements(m_styleOption, m_widget, m_subElementList);
    }
    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
        m_isValid = true;
    } else {
        m_isValid = false;
        qWarning() << "Could not find elementlist for this element!";
    }
}

QSize AbstractElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    qWarning() << "contentsSize is unimplemented for" << m_styleOption;
    return contentsSizeFromStyle;
}

QRect AbstractElement::subElementRect(QStyle::SubElement element) const
{
    qWarning() << "subElementRect is unimplemented for " << element;
    return QRect();
}

QRect AbstractElement::subControlRect(QStyle::SubControl subControl) const
{
    qWarning() << "subControlRect is unimplemented for " << subControl;
    return QRect();
}

void AbstractElement::updateSubElementList()
{
    qWarning() << "updateSubElementList is unimplemented for" << m_widget;
}

QSize AbstractElement::applyPaddingToSize(QSize oldSize, bool shrink) const
{
    if (!m_isValid) {
        return oldSize;
    }
    QSize minimumSize = oldSize;
    QSize size = minimumSize;
    QMargins padding;
    if (m_backgroundProperties->layout()) {
        auto width = m_backgroundProperties->layout()->width().value_or(1);
        auto height = m_backgroundProperties->layout()->height().value_or(1);
        minimumSize = QSize(width, height);
        if (m_backgroundProperties->layout()->padding()) {
            padding = m_backgroundProperties->layout()->padding()->toMargins().toMargins();
        }
        if (m_backgroundProperties->layout()->inset()) {
            padding += m_backgroundProperties->layout()->inset()->toMargins().toMargins();
        }
    }
    if (shrink) {
        size = size.shrunkBy(padding);
        if (size.width() < 0) {
            size.setWidth(0);
        }
        if (size.height() < 0) {
            size.setHeight(0);
        }
    } else {
        size = size.grownBy(padding);
        if (size.width() < minimumSize.width()) {
            size.setWidth(minimumSize.width());
        }
        if (size.height() < minimumSize.height()) {
            size.setHeight(minimumSize.height());
        }
    }
    return size;
}

void AbstractElement::drawBg(QPainter *painter) const
{
    drawBackground(painter, m_styleOption->rect, m_backgroundProperties);
}

void AbstractElement::drawText(QPainter *painter) const
{
    if (hasText() && m_isValid) {
        QRect textRect = m_layoutMap[ElementString::Text].rect.toRect();
        int textFlags = Qt::AlignLeading | Qt::AlignVCenter;
        const bool enabled = m_styleOption->state.testFlag(QStyle::State_Enabled);
        QColor penColor = m_styleOption->palette.text().color();
        if (m_contentProperties->text()) {
            auto textColor = m_contentProperties->text()->color();
            if (textColor) {
                penColor = textColor->toQColor();
            }
            textFlags = textFlagsFromProperties(m_contentProperties, true);
        }
        painter->save();
        if (m_contentProperties->text() && m_contentProperties->text()->font().has_value()) {
            painter->setFont(m_contentProperties->text()->font().value());
        }
        painter->setPen(penColor);
        m_style->drawItemText(painter, textRect, textFlags, m_styleOption->palette, enabled, m_text);
        painter->restore();
    }
}

void AbstractElement::drawIcon(QPainter *painter) const
{
    if (hasIcon() && m_isValid) {
        QRect iconRect = m_layoutMap[ElementString::Icon].rect.toRect();
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

void AbstractElement::drawIndicator(QPainter *painter) const
{
    if (hasIndicator() && m_isValid) {
        QRect indicatorRect = m_layoutMap[ElementString::Indicator].rect.toRect();
        drawBackground(painter, indicatorRect, m_indicatorProperties);
        const bool enabled = m_styleOption->state.testFlag(QStyle::State_Enabled);

        const QPalette activePalette = m_styleOption->palette;
        const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
        auto iconSize = indicatorRect.size();
        const QPixmap pixmap = m_indicator.pixmap(iconSize, dpr, enabled ? QIcon::Normal : QIcon::Disabled);

        QColor penColor = m_styleOption->palette.text().color(); // Use text color as fallback
        if (m_indicatorProperties->icon() && m_indicatorProperties->icon()->color().has_value()) {
            auto iconColor = m_indicatorProperties->icon()->color();
            penColor = iconColor->toQColor();
        }

        painter->save();
        painter->setPen(penColor);
        m_style->drawItemPixmap(painter, indicatorRect, Qt::AlignCenter, pixmap);
        painter->restore();
    }
}