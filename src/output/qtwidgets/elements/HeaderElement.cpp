// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "HeaderElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

HeaderElement::HeaderElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_headerOption(qstyleoption_cast<const QStyleOptionHeader *>(option))
{
    if (m_headerOption) {
        if (!m_headerOption->text.isEmpty()) {
            setText(m_headerOption->text);
        }
    }
    updateSubElementList();
    layout();
}

HeaderElement::~HeaderElement()
{
}

QIcon HeaderElement::sortIndicator()
{
    QIcon sortIndicator;
    if (m_headerOption && m_contentProperties) {
        switch (m_headerOption->sortIndicator) {
        case QStyleOptionHeader::None:
            break;
        case QStyleOptionHeader::SortUp:
            if (m_contentProperties->icon()) {
                sortIndicator = QIcon::fromTheme(m_contentProperties->icon()->name().value_or(u"arrow-up-symbolic"_s));
            }
            break;
        case QStyleOptionHeader::SortDown:
            if (m_contentProperties->icon()) {
                sortIndicator = QIcon::fromTheme(m_contentProperties->icon()->name().value_or(u"arrow-down-symbolic"_s));
            }
            break;
        }
    }
    return sortIndicator;
}

void HeaderElement::layout()
{
    // Background and content is separate
    if (m_backgroundElementList.isEmpty()) {
        m_backgroundElementList = prepareElements(m_styleOption, m_widget, {ElementString::HeaderViewDelegate});
    }
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
        m_layoutMap = layoutMap(m_backgroundElementList, m_styleOption, m_subElementList);
    }

    if (m_contentElementList.isEmpty()) {
        m_contentElementList = prepareElements(m_styleOption, m_widget, {ElementString::HeaderViewDelegate});
    }
    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
        setIcon(sortIndicator());
        m_isValid = true;
    } else {
        m_isValid = false;
        qWarning() << "Could not find elementlist for this element!";
    }
}

void HeaderElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }
    drawBackground(painter);
    drawIcon(painter);
    drawText(painter);
}

void HeaderElement::updateSubElementList()
{
    m_subElementList = {ElementString::Text, ElementString::Icon};
}

QSize HeaderElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    return applyPaddingToSize(contentsSizeFromStyle);
}

QRect HeaderElement::subElementRect(QStyle::SubElement element) const
{
    if (!m_isValid) {
        qWarning() << "Subelementrect for " << element << "is not valid";
        return QRect();
    }
    QRect rect;
    if (element == QStyle::SE_HeaderArrow || element == QStyle::SE_HeaderLabel) {
        auto mapItem = (element == QStyle::SE_HeaderLabel) ? ElementString::Text : ElementString::Icon;
        rect = m_layoutMap[mapItem].rect.toRect();
    }
    return rect;
}
