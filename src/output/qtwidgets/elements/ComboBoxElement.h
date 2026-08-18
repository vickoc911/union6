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

class ComboBoxElement : public AbstractElement
{
    Q_OBJECT

public:
    ComboBoxElement(const QStyleOptionComboBox *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ComboBoxElement() override;

    void draw(QPainter *painter) const override;

    QRect subControlRect(QStyle::SubControl subControl) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    void updateSubElementList() override;

    bool isEditable() const;

private:
    const QStyleOptionComboBox *m_comboBoxOption = nullptr;
    qreal m_spacing;
    bool m_editable;
};
