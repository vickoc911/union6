// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

// This is a kitchen-sink element class to draw any various indicators

class TreeViewElement : public AbstractElement
{
    Q_OBJECT

public:
    TreeViewElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~TreeViewElement() override;

    void draw(QPainter *painter, DrawEnums enums) const override;
    qreal indentation() const;

private:
    void drawIndicator(QPainter *painter) const override;
    const QStyleOption *m_treeViewOption = nullptr;
};
