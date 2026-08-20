// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "TabElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>
#include <QTabBar>

using namespace Qt::StringLiterals;

TabElement::TabElement(const QStyleOptionTab *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_tabOption(option)
    , m_isVertical(false)
    , m_isClosable(false)
{
    update();
}

TabElement::~TabElement()
{
}

void TabElement::update()
{
    m_isVertical = m_tabOption->shape == QTabBar::RoundedEast || m_tabOption->shape == QTabBar::RoundedWest || m_tabOption->shape == QTabBar::TriangularEast
        || m_tabOption->shape == QTabBar::TriangularWest;

    if (const auto tabbarwidget = qobject_cast<const QTabBar *>(m_widget)) {
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

    updateSubElementList();
    layout();
}

void TabElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }
    drawBackground(painter);
    drawIcon(painter);
    drawText(painter);
    drawIndicator(painter);
}

void TabElement::updateSubElementList()
{
    m_subElementList.clear();
    if (m_isClosable) {
        m_subElementList.append(ElementString::CloseButton);
    }
    if (!m_tabOption->icon.isNull()) {
        m_subElementList.append(ElementString::Icon);
    }
    if (!m_tabOption->text.isEmpty()) {
        m_subElementList.append(ElementString::Text);
    }
}

void TabElement::layout()
{
    if (m_subElementList.isEmpty()) {
        m_isValid = false;
        return;
    }
    // Background and content is separate
    m_backgroundElementList = prepareElements(m_tabOption, m_widget, {ElementString::Tab});

    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
    }

        QStringList elements = {ElementString::Tab};
        elements.append(m_subElementList);
        m_contentElementList = prepareElements(m_tabOption, m_widget, elements);

    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
    }

    m_layoutMap = layoutMap(m_backgroundElementList, m_tabOption, m_subElementList);
    m_isValid = true;
}

QSize TabElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    const auto frameSize = m_style->pixelMetric(QStyle::PM_DefaultFrameWidth, m_tabOption, m_widget);
    const auto size = applyPaddingToSize(contentsSizeFromStyle);
    QMargins frameMargins;
    if (m_isVertical) {
        frameMargins = QMargins(frameSize, 0, frameSize, 0);
    } else {
        frameMargins = QMargins(0, frameSize, 0, frameSize);
    }

    return size.grownBy(frameMargins);
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
    return m_tabOption->rect;
}

bool TabElement::isVertical() const
{
    return m_isVertical;
}