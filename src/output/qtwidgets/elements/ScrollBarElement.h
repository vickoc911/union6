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

class ScrollBarElement : public AbstractElement
{
    Q_OBJECT

public:
    ScrollBarElement(const QStyleOptionSlider *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ScrollBarElement() override;

    void update() override;
    void draw(QPainter *painter) const override;

    void layout() override;
    QRect subControlRect(QStyle::SubControl subControl) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    void drawBackground(QPainter *painter) const override;
    void drawIndicator(QPainter *painter) const override;
    void updateSubElementList() override;

private:
    const QStyleOptionSlider *m_scrollBarOption = nullptr;
    bool m_horizontal;
};
