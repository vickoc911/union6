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

class TabElement : public AbstractElement
{
    Q_OBJECT

public:
    TabElement(const QStyleOptionTab *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~TabElement() override;

    void update() override;
    void draw(QPainter *painter) const override;

    void layout() override;
    void updateSubElementList() override;
    QRectF subElementRect(QStyle::SubElement element) const override;
    QSizeF contentsSize(const QSizeF &contentsSizeFromStyle) const override;

    bool isVertical() const;
    int hSpace() const;
    int vSpace() const;

private:
    const QStyleOptionTab *m_tabOption = nullptr;
    bool m_isVertical;
    bool m_isClosable;
    void tabLayout(QRectF *textRect, QRectF *iconRect) const;
};
