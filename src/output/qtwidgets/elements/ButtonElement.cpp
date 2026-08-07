// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ButtonElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

ButtonElement::ButtonElement(ElementType type, const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(type, option, style, widget)
{
    if (const auto buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
        if (buttonOption->features.testFlag(QStyleOptionButton::HasMenu)) { }
        if (!buttonOption->icon.isNull()) {
            setIcon(buttonOption->icon);
        }
        if (!buttonOption->text.isEmpty()) {
            setText(buttonOption->text);
        }
    }
}

ButtonElement::~ButtonElement()
{
}

ButtonElement::Ptr ButtonElement::create(ElementType type, const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
{
    return std::make_shared<ButtonElement>(type, option, style, widget);
}
