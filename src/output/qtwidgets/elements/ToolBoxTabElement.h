// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include "BackgroundDrawing.h"
#include <QIcon>
#include <QObject>
#include <QStyleOption>
#include <qstyleoption.h>

class UnionStyle;

class ToolBoxTabElement : public AbstractElement
{
    Q_OBJECT

public:
    ToolBoxTabElement(const QStyleOptionToolBox *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ToolBoxTabElement() override;

    void update() override;
    void draw(QPainter *painter) const override;

    void layout() override;
    void updateSubElementList() override;
    QRectF subElementRect(QStyle::SubElement element) const override;

private:
    const QStyleOptionToolBox *m_toolBoxOption = nullptr;
};
