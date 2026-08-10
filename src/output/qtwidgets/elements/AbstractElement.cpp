// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "AbstractElement.h"
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
    , m_isValid(false)
{
    updateSubElementList();
    layout();
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
    }

    if (m_contentElementList.isEmpty()) {
        m_contentElementList = prepareElements(m_styleOption, m_widget, m_subElementList);
    }
    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
        m_layoutMap = layoutMap(m_backgroundElementList, m_styleOption, m_subElementList);
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
    m_subElementList = buildSubElementList(m_styleOption, m_widget);
}

QSize AbstractElement::applyPaddingToSize(QSize oldSize) const
{
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
    size = size.grownBy(padding);
    if (size.width() < minimumSize.width()) {
        size.setWidth(minimumSize.width());
    }
    if (size.height() < minimumSize.height()) {
        size.setHeight(minimumSize.height());
    }
    return size;
}

void AbstractElement::drawBg(QPainter *painter) const
{
    drawBackground(painter, m_styleOption->rect, m_backgroundProperties);
}

void AbstractElement::drawText(QPainter *painter) const
{
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

void AbstractElement::drawIcon(QPainter *painter) const
{
    if (hasIcon()) {
        QRect iconRect = m_layoutMap[u"Icon"_s].rect.toRect();
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
