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

class ToolButtonElement : public AbstractElement
{
    Q_OBJECT

public:
    ToolButtonElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ToolButtonElement() override;

    void draw(QPainter *painter) const override;

    QRect subControlRect(QStyle::SubControl subControl) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    const QStyleOptionToolButton *m_toolButtonOption = nullptr;

    void updateSubElementList() override;
    void drawText(QPainter *painter) const override;
    void drawIcon(QPainter *painter) const override;

private:
    bool m_hasIndicator;
    bool m_hasArrows;
    bool m_hasIcon;
    bool m_hasText;
};
