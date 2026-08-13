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

class SliderElement : public AbstractElement, public std::enable_shared_from_this<SliderElement>
{
    Q_OBJECT

public:
    using Ptr = std::shared_ptr<SliderElement>;
    SliderElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~SliderElement() override;
    static SliderElement::Ptr create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget);

    void draw(QPainter *painter) const override;

    QRect subControlRect(QStyle::SubControl subControl) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    const QStyleOptionSlider *m_sliderOption = nullptr;

    void layout() override;

    void drawBg(QPainter *painter) const override;
    void updateSubElementList() override;

private:
    bool m_isHorizontal;
    bool m_isInverted;
    bool m_isReverse;
    QList<QRect> tickLines() const;
};
