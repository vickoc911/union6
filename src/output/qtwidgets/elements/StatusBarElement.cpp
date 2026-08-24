// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "StatusBarElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

StatusBarElement::StatusBarElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_statusBarOption(option)
{
    update();
}

StatusBarElement::~StatusBarElement()
{
}

void StatusBarElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    updateSubElementList();
    layout();
}

void StatusBarElement::updateSubElementList()
{
    m_subElementList = {ElementString::Item};
}

void StatusBarElement::layout()
{
    // Background and content is separate
    m_backgroundElementList = prepareElements(m_styleOption, m_widget);
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
        m_layoutMap = layoutMap(m_backgroundElementList, m_styleOption, m_subElementList);
    }

    m_contentElementList = prepareElements(m_styleOption, m_widget, m_subElementList);
    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
        m_isValid = true;
    } else {
        m_isValid = false;
        qWarning() << "Could not find elementlist for this element!";
    }
}

void StatusBarElement::drawBackground(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_statusBarOption->rect, m_backgroundProperties);
}

void StatusBarElement::drawItem(QPainter *painter) const
{
    drawBackgroundRectangle(painter, m_statusBarOption->rect, m_contentProperties);
}
