// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "CheckElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

CheckElement::CheckElement(Type type, const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_buttonOption(qstyleoption_cast<const QStyleOptionButton *>(option))
    , m_indicatorIcon(QIcon())
    , m_type(type)
{
    if (m_buttonOption) {
        m_indicatorElementList = prepareElements(m_styleOption, m_widget, {u"Indicator"_s});
        if (!m_indicatorElementList.isEmpty()) {
            m_indicatorProperties = queryProperties(m_indicatorElementList);
        }
        if (!m_buttonOption->icon.isNull()) {
            setIcon(m_buttonOption->icon);
        }
        if (!m_buttonOption->text.isEmpty()) {
            setText(m_buttonOption->text);
        }
    }
}

CheckElement::~CheckElement()
{
}

void CheckElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }
    drawBackground(painter, m_styleOption->rect, m_backgroundProperties);
    drawIcon(painter);
    drawText(painter);
    drawIndicator(painter);
}

QSize CheckElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    return contentsSizeFromStyle;
}

QRect CheckElement::subElementRect(QStyle::SubElement element) const
{
    if (!m_isValid) {
        qWarning() << "Subelementrect for " << element << "is not valid";
        return QRect();
    }

    if (element == QStyle::SE_CheckBoxIndicator || element == QStyle::SE_RadioButtonIndicator) {
        auto map = layoutMap(m_backgroundElementList, m_styleOption, {u"Indicator"_s});
        return map[u"Indicator"_s].rect.toRect();
    }

    QRect rect = m_styleOption->rect;
    QRect unifiedRect;
    for (const auto &m : m_layoutMap) {
        if (m.elementName != u"Indicator"_s) {
            unifiedRect = unifiedRect.united(m.rect.toRect());
        }
    }
    rect = unifiedRect;
    return rect;
}

CheckElement::Ptr CheckElement::create(Type type, const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
{
    return std::make_shared<CheckElement>(type, option, style, widget);
}

void CheckElement::drawIndicator(QPainter *painter) const
{
    auto subopt = *m_buttonOption;
    if (m_type == CheckElement::Type::CheckBox) {
        subopt.rect = subElementRect(QStyle::SE_CheckBoxIndicator);
        drawElementBackground(painter, &subopt, m_widget, {u"CheckBox"_s, u"Indicator"_s});
    } else {
        subopt.rect = subElementRect(QStyle::SE_RadioButtonIndicator);
        drawElementBackground(painter, &subopt, m_widget, {u"RadioButton"_s, u"Indicator"_s});
    }
}
