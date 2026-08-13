// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "TitleBarElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

TitleBarElement::TitleBarElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_titleBarOption(qstyleoption_cast<const QStyleOptionTitleBar *>(option))
{
    if (m_titleBarOption) {
        if (!m_titleBarOption->icon.isNull()) {
            setIcon(m_titleBarOption->icon);
        }
        if (!m_titleBarOption->text.isEmpty()) {
            setText(m_titleBarOption->text);
        }
    }
    updateSubElementList();
    layout();
}

TitleBarElement::~TitleBarElement()
{
}

void TitleBarElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }

    drawBg(painter);
    if (!m_titleBarOption) {
        return;
    }
    drawElementBackground(painter, m_titleBarOption, m_widget, {u"TitleBar"_s});
    auto map = layoutMap(prepareElements(m_titleBarOption, m_widget, {u"TitleBar"_s}), m_titleBarOption, buildSubElementList(m_titleBarOption, m_widget));
    if (!m_titleBarOption->text.isEmpty()
        && (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowTitleHint) || m_titleBarOption->titleBarFlags.testFlag(Qt::WindowSystemMenuHint))) {
        m_style->drawText(map[u"Text"_s].rect.toRect(), m_titleBarOption, painter, m_titleBarOption->text, m_widget);
    }
    if (!m_titleBarOption->icon.isNull()) {
        m_style->drawIcon(map[u"Icon"_s].rect.toRect(), m_titleBarOption, painter, m_titleBarOption->icon, m_widget);
    }
    if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowContextHelpButtonHint)) {
        const auto icon = queryIcon(m_titleBarOption, m_widget, u"help-contextual-symbolic"_s, {u"TitleBar"_s, u"HelpButton"_s});
        m_style->drawIcon(map[u"HelpButton"_s].rect.toRect(), m_titleBarOption, painter, icon, m_widget);
    }
    if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowMinimizeButtonHint)) {
        const auto icon = queryIcon(m_titleBarOption, m_widget, u"window-minimize-symbolic"_s, {u"TitleBar"_s, u"MinimizeButton"_s});
        m_style->drawIcon(map[u"MinimizeButton"_s].rect.toRect(), m_titleBarOption, painter, icon, m_widget);
    }
    if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowMaximizeButtonHint)) {
        const auto icon = queryIcon(m_titleBarOption, m_widget, u"window-maximize-symbolic"_s, {u"TitleBar"_s, u"MaximizeButton"_s});
        m_style->drawIcon(map[u"MaximizeButton"_s].rect.toRect(), m_titleBarOption, painter, icon, m_widget);
    }
    if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowCloseButtonHint)) {
        const auto icon = queryIcon(m_titleBarOption, m_widget, u"window-close-symbolic"_s, {u"TitleBar"_s, u"CloseButton"_s});
        m_style->drawIcon(map[u"CloseButton"_s].rect.toRect(), m_titleBarOption, painter, icon, m_widget);
    }
    if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowSystemMenuHint)) {
        const auto icon = queryIcon(m_titleBarOption, m_widget, u"application-menu-symbolic"_s, {u"TitleBar"_s, u"SystemMenu"_s});
        m_style->drawIcon(map[u"SystemMenu"_s].rect.toRect(), m_titleBarOption, painter, icon, m_widget);
    }
    if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowShadeButtonHint)) {
        const auto icon = queryIcon(m_titleBarOption, m_widget, u"window-shade-symbolic"_s, {u"TitleBar"_s, u"ShadeButton"_s});
        m_style->drawIcon(map[u"ShadeButton"_s].rect.toRect(), m_titleBarOption, painter, icon, m_widget);
    }
}

void TitleBarElement::updateSubElementList()
{
    m_subElementList.clear();
    if (m_titleBarOption) {
        if (!m_titleBarOption->text.isEmpty()
            && (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowTitleHint) || m_titleBarOption->titleBarFlags.testFlag(Qt::WindowSystemMenuHint))) {
            m_subElementList.append(u"Text"_s);
        }
        if (!m_titleBarOption->icon.isNull()) {
            m_subElementList.append(u"Icon"_s);
        }
        if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowContextHelpButtonHint)) {
            m_subElementList.append(u"HelpButton"_s);
        }
        if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowMinimizeButtonHint)) {
            m_subElementList.append(u"MinimizeButton"_s);
        }
        if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowMaximizeButtonHint)) {
            m_subElementList.append(u"MaximizeButton"_s);
        }
        if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowCloseButtonHint)) {
            m_subElementList.append(u"CloseButton"_s);
        }
        if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowSystemMenuHint)) {
            m_subElementList.append(u"SystemMenu"_s);
        }
        if (m_titleBarOption->titleBarFlags.testFlag(Qt::WindowShadeButtonHint)) {
            m_subElementList.append(u"ShadeButton"_s);
        }
    }
}

void TitleBarElement::layout()
{
    // Background and content is separate
    if (m_backgroundElementList.isEmpty()) {
        m_backgroundElementList = prepareElements(m_styleOption, m_widget, {u"TitleBar"_s});
    }
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
        m_layoutMap = layoutMap(m_backgroundElementList, m_styleOption, m_subElementList);
    }

    if (m_contentElementList.isEmpty()) {
        m_contentElementList = prepareElements(m_styleOption, m_widget, {u"TitleBar"_s});
    }
    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
        m_isValid = true;
    } else {
        m_isValid = false;
        qWarning() << "Could not find elementlist for this element!";
    }
}

QSize TitleBarElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    return contentsSizeFromStyle;
}

QRect TitleBarElement::subControlRect(QStyle::SubControl subControl) const
{
    if (!m_isValid) {
        qWarning() << "subControlRect for " << subControl << "is not valid";
        return QRect();
    }

    if (!m_titleBarOption) {
        return QRect();
    }

    switch (subControl) {
    case QStyle::SC_TitleBarSysMenu:
        return m_layoutMap[u"SystemMenu"_s].rect.toRect();
    case QStyle::SC_TitleBarMinButton:
        return m_layoutMap[u"MinimizeButton"_s].rect.toRect();
    case QStyle::SC_TitleBarMaxButton:
        return m_layoutMap[u"MaximizeButton"_s].rect.toRect();
    case QStyle::SC_TitleBarCloseButton:
        return m_layoutMap[u"CloseButton"_s].rect.toRect();
    case QStyle::SC_TitleBarNormalButton:
        return m_layoutMap[u"NormalButton"_s].rect.toRect();
    case QStyle::SC_TitleBarShadeButton:
    case QStyle::SC_TitleBarUnshadeButton:
        return m_layoutMap[u"ShadeButton"_s].rect.toRect();
    case QStyle::SC_TitleBarContextHelpButton:
        return m_layoutMap[u"HelpButton"_s].rect.toRect();
    case QStyle::SC_TitleBarLabel:
        return m_layoutMap[u"Text"_s].rect.toRect();
        break;
    default:
        break;
    }
    return QRect();
}

TitleBarElement::Ptr TitleBarElement::create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
{
    return std::make_shared<TitleBarElement>(option, style, widget);
}
