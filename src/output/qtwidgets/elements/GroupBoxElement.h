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

class GroupBoxElement : public AbstractElement
{
    Q_OBJECT

public:
    GroupBoxElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~GroupBoxElement() override;

    void draw(QPainter *painter) const override;

    void layout() override;
    QRect subControlRect(QStyle::SubControl subControl) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    const QStyleOptionGroupBox *m_groupBoxOption = nullptr;

    void drawText(QPainter *painter) const override;
    void drawIcon(QPainter *painter) const override;

private:
    bool m_isCheckable;
};
