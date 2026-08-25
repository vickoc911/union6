// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "TabCloseButtonElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

TabCloseButtonElement::TabCloseButtonElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_widgetOption(option)
{
    update();
}

TabCloseButtonElement::~TabCloseButtonElement()
{
}

void TabCloseButtonElement::update()
{
    layout();
}

void TabCloseButtonElement::layout()
{
    m_contentElementList = prepareElements(m_styleOption, m_widget, {ElementString::Tab, ElementString::CloseButton});
    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
        m_layoutMap[ElementString::Icon].rect = m_widgetOption->rect;
        if (m_contentProperties->icon()) {
            setIcon(QIcon::fromTheme(m_contentProperties->icon()->name().value_or(u"tab-close-symbolic"_s)));
        } else {
            setIcon(QIcon::fromTheme(u"tab-close-symbolic"_s));
        }
        m_isValid = true;
    }
}
