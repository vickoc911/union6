// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "WidgetElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

WidgetElement::WidgetElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_widgetOption(option)
{
    update();
}

WidgetElement::~WidgetElement()
{
}

void WidgetElement::update()
{
    layout();
}

QVariantMap WidgetElement::elementAttributes() const
{
    QVariantMap map;
    // Custom KDE widget. It uses PE_Widget for drawing, so
    // handle it in here.
    if (m_widget && m_widget->inherits("KMultiTabBar")) {
        enum class Position {
            Left,
            Right,
            Top,
            Bottom,
        };

        const Position position = static_cast<Position>(m_widget->property("position").toInt());

        switch (position) {
        case Position::Left:
            map[u"direction"_s] = u"left"_s;
            break;
        case Position::Right:
            map[u"direction"_s] = u"right"_s;
            break;
        case Position::Top:
            map[u"direction"_s] = u"top"_s;
            break;
        case Position::Bottom:
            map[u"direction"_s] = u"bottom"_s;
            break;
        }
    }
    return map;
}

void WidgetElement::draw(QPainter *painter, DrawEnums enums) const
{
    if (!m_isValid) {
        return;
    }
    switch (enums.PrimitiveElement) {
    case QStyle::PE_Widget:
        drawBackground(painter);
        break;
    }
}
