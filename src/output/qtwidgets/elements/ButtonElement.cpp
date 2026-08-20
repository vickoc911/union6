// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ButtonElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

ButtonElement::ButtonElement(const QStyleOptionButton *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_buttonOption(option)
{
    update();
}

ButtonElement::~ButtonElement()
{
}

void ButtonElement::update()
{
    setIndicator(QIcon());
    if (m_buttonOption->features.testFlag(QStyleOptionButton::HasMenu)) {
        m_indicatorElementList = prepareElements(m_styleOption, m_widget, {ElementString::Indicator});
        if (!m_indicatorElementList.isEmpty()) {
            m_indicatorProperties = queryProperties(m_indicatorElementList);
            if (m_indicatorProperties->icon()) {
                setIndicator(QIcon::fromTheme(m_indicatorProperties->icon()->name().value_or(QString())));
            }
        }
    }

    setIcon(m_buttonOption->icon);
    setText(m_buttonOption->text);

    updateSubElementList();
    layout();
}

void ButtonElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }
    drawBackground(painter);
    drawIcon(painter);
    drawText(painter);
    drawIndicator(painter);
}

void ButtonElement::updateSubElementList()
{
    m_subElementList.clear();
    if (m_buttonOption) {
        if (m_buttonOption->features.testFlag(QStyleOptionButton::HasMenu)) {
            m_subElementList.append(ElementString::Indicator);
        }
        if (!m_buttonOption->icon.isNull()) {
            m_subElementList.append(ElementString::Icon);
        }
        if (!m_buttonOption->text.isEmpty()) {
            m_subElementList.append(ElementString::Text);
        }
    }
}

QSize ButtonElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    Q_UNUSED(contentsSizeFromStyle);
    QSize size = subElementRect(QStyle::SE_PushButtonContents).size();
    size = applyPaddingToSize(size);
    // Since text and icon are parts of background, we need to apply the indicator width and spacing from background
    // to get the proper contentSize
    if (hasIndicator()) {
        qreal spacing = 0;
        if (m_backgroundProperties->layout()) {
            spacing = m_backgroundProperties->layout()->spacing().value_or(0);
        }
        size.rwidth() += m_layoutMap[ElementString::Indicator].rect.width() + spacing;
    }
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
