// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include "BackgroundDrawing.h"
#include <QIcon>
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class SpinBoxElement : public AbstractElement
{
    Q_OBJECT

public:
    SpinBoxElement(const QStyleOptionSpinBox *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~SpinBoxElement() override;

    void update() override;
    void draw(QPainter *painter) const override;

    QRectF subControlRect(QStyle::SubControl subControl) const override;
    QSizeF contentsSize(const QSizeF &contentsSizeFromStyle) const override;

    void updateSubElementList() override;

    void drawSpinIndicator(QPainter *painter, const QStyle::PrimitiveElement &primitive, const QRectF &rect) const;

    QStringList elementHints() const override;

private:
    const QStyleOptionSpinBox *m_spinBoxOption = nullptr;
    bool m_hasButtons;
};
