// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ToolBarElement.h"
#include "BackgroundDrawing.h"
#include "StyleUtils.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

#include "SharedNames.h"

using namespace Qt::StringLiterals;

ToolBarElement::ToolBarElement(const QStyleOptionToolBar *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_toolBarOption(option)
{
    update();
}

ToolBarElement::~ToolBarElement()
{
}

void ToolBarElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    updateSubElementList();
    layout();
}

void ToolBarElement::layout()
{
    // Background and content is separate
    m_backgroundElementList = prepareElements(m_toolBarOption, m_widget);
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
        m_layoutMap = layoutMap(m_backgroundElementList, m_toolBarOption, m_subElementList);
    }

    m_handleElementList = prepareElements(m_toolBarOption, m_widget, {ElementString::Handle});
    if (!m_handleElementList.isEmpty()) {
        m_handleProperties = queryProperties(m_handleElementList);
    }
    m_separatorElementList = prepareElements(m_toolBarOption, m_widget, {ElementString::Separator});
    if (!m_separatorElementList.isEmpty()) {
        m_separatorProperties = queryProperties(m_handleElementList);
    }

    m_contentElementList = prepareElements(m_toolBarOption, m_widget, m_subElementList);
    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
        m_isValid = true;
    } else {
        m_isValid = false;
        qCWarning(UNION_QTWIDGETS) << "Could not find elementlist for this element!";
    }
}

void ToolBarElement::drawBackground(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_toolBarOption->rect, m_backgroundProperties);
}

void ToolBarElement::drawFrame(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_toolBarOption->rect, m_backgroundProperties, BackgroundParts::FrameOnly);
}

void ToolBarElement::drawHandle(QPainter *painter) const
{
    if (m_handleProperties && m_handleProperties->layout()) {
        drawBackgroundRectangle(painter, m_toolBarOption->rect, m_handleProperties);
    }
}

void ToolBarElement::drawSeparator(QPainter *painter) const
{
    if (m_separatorProperties && m_separatorProperties->layout()) {
        drawBackgroundRectangle(painter, m_toolBarOption->rect, m_separatorProperties);
    }
}

void ToolBarElement::updateSubElementList()
{
    m_subElementList.clear();
    m_subElementList.append(ElementString::Frame);
}
