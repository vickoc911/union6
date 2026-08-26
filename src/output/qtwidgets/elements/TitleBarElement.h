// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include "BackgroundDrawing.h"
#include <QIcon>
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class TitleBarElement : public AbstractElement
{
    Q_OBJECT

public:
    TitleBarElement(const QStyleOptionTitleBar *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~TitleBarElement() override;

    void update() override;
    void draw(QPainter *painter) const override;

    QRectF subControlRect(QStyle::SubControl subControl) const override;
    QSizeF contentsSize(const QSizeF &contentsSizeFromStyle) const override;

    void updateSubElementList() override;
    void layout() override;

    qreal buttonWidth() const;
    void drawButton(QPainter *painter, const QRectF &rect, const QIcon &icon) const;

    QStringList elementHints() const override;

private:
    const QStyleOptionTitleBar *m_titleBarOption = nullptr;

    QIcon queryIcon(const QString &defaultIconName, const QStringList &targetHierarchy) const;
};
