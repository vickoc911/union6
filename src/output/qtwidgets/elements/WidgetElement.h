// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class WidgetElement : public AbstractElement
{
    Q_OBJECT

public:
    WidgetElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~WidgetElement() override;

    void update() override;
    void draw(QPainter *painter, DrawEnums enums) const override;

private:
    const QStyleOption *m_widgetOption = nullptr;
};
