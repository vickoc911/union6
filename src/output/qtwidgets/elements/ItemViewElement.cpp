// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ItemViewElement.h"
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
            m_indicatorElementList = prepareElements(m_styleOption, m_widget, {u"CheckBox"_s});
            if (!m_indicatorElementList.isEmpty()) {
                m_indicatorProperties = queryProperties(m_indicatorElementList);
                if (m_indicatorProperties->icon()) {
                    setIndicator(QIcon::fromTheme(m_indicatorProperties->icon()->name().value_or(QString())));
                }
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
        m_backgroundElementList = prepareElements(m_styleOption, m_widget, {u"ItemViewItem"_s});
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
        checkbox.rect = m_style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, m_viewItemOption, m_widget);
        painter->save();
        m_style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &checkbox, painter, m_widget);
        painter->restore();
    }
}

void ItemViewElement::updateSubElementList()
{
    m_subElementList.clear();
    if (m_viewItemOption) {
        m_subElementList.append(u"ItemViewItem"_s);
        if (m_viewItemOption->features.testFlag(QStyleOptionViewItem::HasCheckIndicator)) {
            m_subElementList.append(u"CheckBox"_s);
        }
        if (!m_viewItemOption->icon.isNull()) {
            m_subElementList.append(u"Icon"_s);
        }
        if (!m_viewItemOption->text.isEmpty()) {
            m_subElementList.append(u"Text"_s);
        }
    }
}

QSize ItemViewElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    return applyPaddingToSize(contentsSizeFromStyle);
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
    auto elements = prepareElements(m_viewItemOption, m_widget, {u"ItemViewItem"_s});
    auto map = layoutMap(elements, m_viewItemOption, m_subElementList);

    QRect rect;
    if (element == QStyle::SE_ItemViewItemText) {
        rect = map[u"Text"_s].rect.toRect();
    }
    if (element == QStyle::SE_ItemViewItemDecoration) {
        rect = map[u"Icon"_s].rect.toRect();
    }
    if (element == QStyle::SE_ItemViewItemCheckIndicator) {
        rect = map[u"CheckBox"_s].rect.toRect();
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
        QRect textRect = m_layoutMap[u"Text"_s].rect.toRect();
        int textFlags = Qt::AlignLeading | Qt::AlignVCenter;
        const bool enabled = m_styleOption->state.testFlag(QStyle::State_Enabled);
        QColor penColor = m_styleOption->palette.text().color();
        // TODO: hide mnemonics if requested
        if (m_backgroundProperties->text()) {
            auto textColor = m_backgroundProperties->text()->color();
            if (textColor) {
                penColor = textColor->toQColor();
            }
            textFlags = textFlagsFromProperties(m_backgroundProperties, false);
        }

        painter->save();
        painter->setPen(penColor);
        m_style->drawItemText(painter, textRect, textFlags, m_styleOption->palette, enabled, m_text);
        painter->restore();
    }
}