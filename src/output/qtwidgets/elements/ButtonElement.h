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

class ButtonElement : public AbstractElement
{
    Q_OBJECT

public:
    ButtonElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ButtonElement() override;

    void draw(QPainter *painter) const override;

    void updateSubElementList() override;
    QRect subElementRect(QStyle::SubElement element) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    const QStyleOptionButton *m_buttonOption = nullptr;
};
