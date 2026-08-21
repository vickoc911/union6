// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include "BackgroundDrawing.h"
#include "StyleUtils.h"
#include <QIcon>
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class MenuBarElement : public AbstractElement
{
    Q_OBJECT

public:
    MenuBarElement(const QStyleOptionMenuItem *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~MenuBarElement() override;

    void update() override;
    void drawBackground(QPainter *painter) const override;

private:
    const QStyleOptionMenuItem *m_menuItemOption = nullptr;
};
