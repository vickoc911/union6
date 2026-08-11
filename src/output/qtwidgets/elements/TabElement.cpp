// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "TabElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>
#include <QTabBar>

using namespace Qt::StringLiterals;

TabElement::TabElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_tabOption(qstyleoption_cast<const QStyleOptionTab *>(option))
    , m_isVertical(false)
    , m_isClosable(false)
{
    if (m_tabOption) {
        m_isVertical = m_tabOption->shape == QTabBar::RoundedEast || m_tabOption->shape == QTabBar::RoundedWest || m_tabOption->shape == QTabBar::TriangularEast
            || m_tabOption->shape == QTabBar::TriangularWest;

        if (const auto tabbarwidget = qobject_cast<const QTabBar *>(widget)) {
            if (tabbarwidget->tabsClosable()) {
                m_isClosable = true;
            }
        }

        if (!m_tabOption->icon.isNull()) {
            setIcon(m_tabOption->icon);
        }
        if (!m_tabOption->text.isEmpty()) {
            setText(m_tabOption->text);
        }
    }
    updateSubElementList();
    layout();
}

TabElement::~TabElement()
{
}

void TabElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }
    drawBg(painter);
    drawIcon(painter);
    drawText(painter);
    drawIndicator(painter);
}

void TabElement::updateSubElementList()
{
    m_subElementList.clear();
    if (m_tabOption) {
        if (m_isClosable) {
            m_subElementList.append(u"CloseButton"_s);
        }
        if (!m_tabOption->icon.isNull()) {
            m_subElementList.append(u"Icon"_s);
        }
        if (!m_tabOption->text.isEmpty()) {
            m_subElementList.append(u"Text"_s);
        }
    }
}

void TabElement::layout()
{
    if (m_subElementList.isEmpty()) {
        m_isValid = false;
        return;
    }
    // Background and content is separate
    if (m_backgroundElementList.isEmpty()) {
        m_backgroundElementList = prepareElements(m_styleOption, m_widget, {u"TabButton"_s});
    }
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
    }

    if (m_contentElementList.isEmpty()) {
        QStringList elements = {u"TabButton"_s};
        elements.append(m_subElementList);
        m_contentElementList = prepareElements(m_styleOption, m_widget, elements);
    }
    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
    }

    m_layoutMap = layoutMap(m_backgroundElementList, m_styleOption, m_subElementList);
    m_isValid = true;
}

QSize TabElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    QSize size = contentsSizeFromStyle;
    size = applyPaddingToSize(size);
    return size;
}

QRect TabElement::subElementRect(QStyle::SubElement element) const
{
    if (!m_isValid) {
        return QRect();
    }

    if (element == QStyle::SE_TabBarTabText) {
        QRect unifiedRect;
        for (const auto &m : m_layoutMap) {
            unifiedRect = unifiedRect.united(m.rect.toRect());
        }
        unifiedRect.setSize(applyPaddingToSize(unifiedRect.size()));
        if (m_isVertical) {
            unifiedRect = unifiedRect.transposed();
        }
    }
    return m_styleOption->rect;
}

TabElement::Ptr TabElement::create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
{
    return std::make_shared<TabElement>(option, style, widget);
}

bool TabElement::isVertical() const
{
    return m_isVertical;
}