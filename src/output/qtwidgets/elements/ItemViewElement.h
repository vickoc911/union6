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

class ItemViewElement : public AbstractElement
{
    Q_OBJECT

public:
    ItemViewElement(const QStyleOptionViewItem *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ItemViewElement() override;

    void update() override;

    void updateSubElementList() override;
    QRect subElementRect(QStyle::SubElement element) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;
    void layout() override;
    void drawIndicator(QPainter *painter) const override;
    void drawText(QPainter *painter) const override;
    void drawIcon(QPainter *painter) const override;

private:
    const QStyleOptionViewItem *m_viewItemOption = nullptr;
};
