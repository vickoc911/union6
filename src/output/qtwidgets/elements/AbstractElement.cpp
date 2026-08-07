// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "AbstractElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

AbstractElement::AbstractElement(ElementType type, const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : QObject(nullptr)
    , m_type(type)
    , m_styleOption(option)
    , m_style(style)
    , m_widget(widget)
    , m_icon(QIcon())
    , m_text(QString())
{
    m_elementList = prepareElements(option, widget);
    if (!m_elementList.isEmpty()) {
        m_properties = queryProperties(m_elementList);
        layout();
    } else {
        m_type = ElementType::Invalid;
        qWarning() << "Could not find elementlist for this element!";
    }
}

AbstractElement::~AbstractElement()
{
}

AbstractElement::Ptr AbstractElement::create(ElementType type, const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
{
    return std::make_shared<AbstractElement>(type, option, style, widget);
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

AbstractElement::ElementType AbstractElement::type() const
{
    return m_type;
}

void AbstractElement::draw(QPainter *painter) const
{
    if (m_type == ElementType::Invalid) {
        return;
    }
    drawBackground(painter, m_styleOption->rect, m_properties);
    drawIcon(painter);
    drawText(painter);
}

void AbstractElement::layout()
{
    auto subElements = buildSubElementList(m_styleOption, m_widget);
    m_layoutMap = layoutMap(m_elementList, m_styleOption, subElements);
}

void AbstractElement::drawText(QPainter *painter) const
{
    if (hasText()) {
        QRect textRect = m_layoutMap[u"Text"_s].rect.toRect();
        int textFlags = Qt::AlignLeading | Qt::AlignVCenter;
        const bool enabled = m_styleOption->state.testFlag(QStyle::State_Enabled);
        QColor penColor = m_styleOption->palette.text().color();
        // TODO: hide mnemonics if requested
        if (m_properties->text()) {
            auto textColor = m_properties->text()->color();
            if (textColor) {
                penColor = textColor->toQColor();
            }
            textFlags = textFlagsFromProperties(m_properties, true);
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
        if (m_properties->icon() && m_properties->icon()->color().has_value()) {
            auto iconColor = m_properties->icon()->color();
            penColor = iconColor->toQColor();
        }

        painter->save();
        painter->setPen(penColor);
        m_style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
        painter->restore();
    }
}
