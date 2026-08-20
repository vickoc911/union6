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

class SliderElement : public AbstractElement
{
    Q_OBJECT

public:
    SliderElement(const QStyleOptionSlider *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~SliderElement() override;

    void update() override;
    void draw(QPainter *painter) const override;

    QRect subControlRect(QStyle::SubControl subControl) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    void layout() override;

    void drawBackground(QPainter *painter) const override;
    void updateSubElementList() override;

private:
    const QStyleOptionSlider *m_sliderOption = nullptr;
    bool m_isHorizontal;
    bool m_isInverted;
    bool m_isReverse;
    QList<QRect> tickLines() const;
};
