// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include "BackgroundDrawing.h"
#include <QIcon>
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class ComboBoxElement : public AbstractElement
{
    Q_OBJECT

public:
    ComboBoxElement(const QStyleOptionComboBox *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ComboBoxElement() override;

    void update() override;

    void draw(QPainter *painter, DrawEnums enums) const override;
    QRectF subControlRect(QStyle::SubControl subControl) const override;
    QSizeF contentsSize(const QSizeF &contentsSizeFromStyle) const override;

private:
    bool isEditable() const;
    QStringList elementHints() const override;
    void updateSubElementList() override;
    void drawText(QPainter *painter) const override;
    const QStyleOptionComboBox *m_comboBoxOption = nullptr;
    qreal m_spacing;
    bool m_editable;
};
