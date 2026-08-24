// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "DockWidgetElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

DockWidgetElement::DockWidgetElement(const QStyleOptionDockWidget *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_dockWidgetOption(option)
{
    update();
}

DockWidgetElement::~DockWidgetElement()
{
}

void DockWidgetElement::update()
{
    setIcon(QIcon());
    setText(m_dockWidgetOption->title);
    updateSubElementList();
    layout();
}

void DockWidgetElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }

    drawBackground(painter);
    drawText(painter);
}

void DockWidgetElement::updateSubElementList()
{
    m_subElementList.clear();
    if (m_dockWidgetOption->closable) {
        m_subElementList.append(ElementString::CloseButton);
    }
    if (m_dockWidgetOption->floatable) {
        m_subElementList.append(ElementString::FloatButton);
    }
    if (!m_dockWidgetOption->title.isEmpty()) {
        m_subElementList.append(ElementString::Text);
    }
}

void DockWidgetElement::layout()
{
    m_backgroundElementList = prepareElements(m_dockWidgetOption, m_widget, {ElementString::DockWidget});

    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
        m_layoutMap = layoutMap(m_backgroundElementList, m_dockWidgetOption, m_subElementList);
    }

    m_contentElementList = prepareElements(m_dockWidgetOption, m_widget, {ElementString::DockWidget});

    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
        m_isValid = true;
    } else {
        m_isValid = false;
        qWarning() << "Could not find elementlist for this element!";
    }
}

QRect DockWidgetElement::subElementRect(QStyle::SubElement subElement) const
{
    if (!m_isValid) {
        return QRect();
    }

    if (subElement == QStyle::SE_DockWidgetTitleBarText) {
        return m_layoutMap[ElementString::Text].rect.toRect();
    }
    if (subElement == QStyle::SE_DockWidgetFloatButton) {
        return m_layoutMap[ElementString::FloatButton].rect.toRect();
    }
    if (subElement == QStyle::SE_DockWidgetCloseButton) {
        return m_layoutMap[ElementString::CloseButton].rect.toRect();
    }
    // The styleoption has no icon, yet there is whole thing for an icon? Wtf.
    if (subElement == QStyle::SE_DockWidgetIcon) {
        return m_layoutMap[ElementString::Icon].rect.toRect();
    }
    return QRect();
}
