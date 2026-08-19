// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ItemViewElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

ItemViewElement::ItemViewElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_viewItemOption(qstyleoption_cast<const QStyleOptionViewItem *>(option))
{
    if (m_viewItemOption) {
        if (m_viewItemOption->features.testFlag(QStyleOptionViewItem::HasCheckIndicator)) {
            m_indicatorElementList = prepareElements(m_styleOption, m_widget, {ElementString::CheckBox});
            if (!m_indicatorElementList.isEmpty()) {
                m_indicatorProperties = queryProperties(m_indicatorElementList);
            }
        }

        if (!m_viewItemOption->icon.isNull() && m_viewItemOption->features.testFlag(QStyleOptionViewItem::HasDecoration)) {
            setIcon(m_viewItemOption->icon);
        }
        if (!m_viewItemOption->text.isEmpty() && m_viewItemOption->features.testFlag(QStyleOptionViewItem::HasDisplay)) {
            setText(m_viewItemOption->text);
        }
    }
    updateSubElementList();
    layout();
}

ItemViewElement::~ItemViewElement()
{
}

void ItemViewElement::layout()
{
    // Background and content is separate
    if (m_backgroundElementList.isEmpty()) {
        m_backgroundElementList = prepareElements(m_styleOption, m_widget, {ElementString::ItemViewItem});
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

void ItemViewElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }
    drawBg(painter);
    drawIcon(painter);
    drawText(painter);
    drawIndicator(painter);
}

void ItemViewElement::drawIndicator(QPainter *painter) const
{
    // Draw indicator
    if (m_viewItemOption && m_viewItemOption->features.testFlag(QStyleOptionViewItem::HasCheckIndicator)) {
        QStyleOptionButton checkbox;
        switch (m_viewItemOption->checkState) {
        case Qt::Unchecked:
            checkbox.state.setFlag(QStyle::State_Off);
            break;
        case Qt::PartiallyChecked:
            checkbox.state.setFlag(QStyle::State_NoChange);
            break;
        case Qt::Checked:
            checkbox.state.setFlag(QStyle::State_On);
            break;
        }
        checkbox.state.setFlag(QStyle::State_Enabled, m_viewItemOption->state.testFlag(QStyle::State_Enabled));
        auto checkBoxRect = m_style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, m_viewItemOption, m_widget);
        // Use iconSize to make sure the checkbox is correct size here
        if (m_contentProperties && m_contentProperties->icon()) {
            checkBoxRect = centerRect(checkBoxRect, m_contentProperties->icon()->width().value_or(0), m_contentProperties->icon()->height().value_or(0));
        }
        checkbox.rect = checkBoxRect;
        painter->save();
        m_style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &checkbox, painter, m_widget);
        painter->restore();
    }
}

void ItemViewElement::updateSubElementList()
{
    m_subElementList.clear();
    if (m_viewItemOption) {
        m_subElementList.append(ElementString::ItemViewItem);
        if (m_viewItemOption->features.testFlag(QStyleOptionViewItem::HasCheckIndicator)) {
            m_subElementList.append(ElementString::CheckBox);
        }
        if (!m_viewItemOption->icon.isNull()) {
            m_subElementList.append(ElementString::Icon);
        }
        if (!m_viewItemOption->text.isEmpty()) {
            m_subElementList.append(ElementString::Text);
        }
    }
}

QSize ItemViewElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    Q_UNUSED(contentsSizeFromStyle);
    const auto textSize = subElementRect(QStyle::SE_ItemViewItemText).size();
    const auto decorationSize = subElementRect(QStyle::SE_ItemViewItemDecoration).size();
    const auto checkboxSize = subElementRect(QStyle::SE_ItemViewItemCheckIndicator).size();
    const auto combinedSize = textSize.expandedTo(decorationSize.expandedTo(checkboxSize));
    return applyPaddingToSize(combinedSize);
}

QRect ItemViewElement::subElementRect(QStyle::SubElement element) const
{
    if (!m_isValid) {
        qWarning() << "Subelementrect for " << element << "is not valid";
        return QRect();
    }

    if (m_subElementList.isEmpty()) {
        return QRect();
    }

    QRect rect;
    if (element == QStyle::SE_ItemViewItemText) {
        rect = m_layoutMap[ElementString::Text].rect.toRect();
    }
    if (element == QStyle::SE_ItemViewItemDecoration) {
        // DecorationSize can be changed by user, so use it by default
        rect = centerRect(m_layoutMap[ElementString::Icon].rect.toRect(), m_viewItemOption->decorationSize.width(), m_viewItemOption->decorationSize.height());
    }
    if (element == QStyle::SE_ItemViewItemCheckIndicator) {
        rect = m_layoutMap[ElementString::CheckBox].rect.toRect();
    }
    return rect;
}

ItemViewElement::Ptr ItemViewElement::create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
{
    return std::make_shared<ItemViewElement>(option, style, widget);
}

void ItemViewElement::drawText(QPainter *painter) const
{
    if (hasText() && m_isValid) {
        QRect textRect = m_style->subElementRect(QStyle::SE_ItemViewItemText, m_viewItemOption, m_widget);
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

void ItemViewElement::drawIcon(QPainter *painter) const
{
    if (hasIcon() && m_isValid) {
        QRect iconRect = m_style->subElementRect(QStyle::SE_ItemViewItemDecoration, m_viewItemOption, m_widget);
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
        if (m_backgroundProperties->icon() && m_backgroundProperties->icon()->color().has_value()) {
            auto iconColor = m_backgroundProperties->icon()->color();
            penColor = iconColor->toQColor();
        }

        painter->save();
        painter->setPen(penColor);
        m_style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
        painter->restore();
    }
}