// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ButtonElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

ButtonElement::ButtonElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_buttonOption(qstyleoption_cast<const QStyleOptionButton *>(option))
    , m_indicatorIcon(QIcon())
{
    if (m_buttonOption) {
        if (m_buttonOption->features.testFlag(QStyleOptionButton::HasMenu)) {
            m_indicatorElements = prepareElements(m_styleOption, m_widget, {u"Indicator"_s});
            if (!m_indicatorElements.isEmpty()) {
                m_indicatorProperties = queryProperties(m_indicatorElements);
                if (m_indicatorProperties->icon()) {
                    m_indicatorIcon = QIcon::fromTheme(m_indicatorProperties->icon()->name().value_or(QString()));
                }
            }
        }

        if (!m_buttonOption->icon.isNull()) {
            setIcon(m_buttonOption->icon);
        }
        if (!m_buttonOption->text.isEmpty()) {
            setText(m_buttonOption->text);
        }
    }
}

ButtonElement::~ButtonElement()
{
}

void ButtonElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }
    drawBackground(painter, m_styleOption->rect, m_backgroundProperties);
    drawIcon(painter);
    drawText(painter);
    drawIndicator(painter);
}

QSize ButtonElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    Q_UNUSED(contentsSizeFromStyle);
    QSize size = subElementRect(QStyle::SE_PushButtonContents).size();
    size = applyPaddingToSize(size);
    return size;
}

QRect ButtonElement::subElementRect(QStyle::SubElement element) const
{
    if (!m_isValid) {
        qWarning() << "Subelementrect for " << element << "is not valid";
        return QRect();
    }

    if (element == QStyle::SE_PushButtonBevel || element == QStyle::SE_PushButtonFocusRect) {
        return backgroundRectangle(m_styleOption, m_backgroundProperties).toRect();
    }

    QRect rect = m_styleOption->rect;
    QRect unifiedRect;
    for (const auto &m : m_layoutMap) {
        unifiedRect = unifiedRect.united(m.rect.toRect());
    }
    rect = unifiedRect;
    return rect;
}

ButtonElement::Ptr ButtonElement::create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
{
    return std::make_shared<ButtonElement>(option, style, widget);
}

void ButtonElement::drawIndicator(QPainter *painter) const
{
    if (!m_indicatorIcon.isNull()) {
        QRect iconRect = m_layoutMap[u"Indicator"_s].rect.toRect();
        const bool enabled = m_styleOption->state.testFlag(QStyle::State_Enabled);

        const QPalette activePalette = m_styleOption->palette;
        const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
        auto iconSize = iconRect.size();
        const QPixmap pixmap = m_indicatorIcon.pixmap(iconSize, dpr, enabled ? QIcon::Normal : QIcon::Disabled);

        QColor penColor = m_styleOption->palette.text().color(); // Use text color as fallback
        if (m_indicatorProperties->icon() && m_indicatorProperties->icon()->color().has_value()) {
            auto iconColor = m_indicatorProperties->icon()->color();
            penColor = iconColor->toQColor();
        }

        painter->save();
        painter->setPen(penColor);
        m_style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
        painter->restore();
    }
}
