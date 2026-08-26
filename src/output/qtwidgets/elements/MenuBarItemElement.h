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

class MenuBarItemElement : public AbstractElement
{
    Q_OBJECT

public:
    MenuBarItemElement(const QStyleOptionMenuItem *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~MenuBarItemElement() override;

    void update() override;
    void updateSubElementList() override;
    void layout() override;

    QStringList elementHints() const override;

private:
    const QStyleOptionMenuItem *m_menuItemOption = nullptr;
};
