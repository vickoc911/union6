// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include "BackgroundDrawing.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class MenuElement : public AbstractElement
{
    Q_OBJECT

public:
    MenuElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~MenuElement() override;

    void update() override;
    void updateSubElementList() override;
    void drawBackground(QPainter *painter) const override;
    void drawFrame(QPainter *painter) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

private:
    const QStyleOption *m_menuOption = nullptr;
};
