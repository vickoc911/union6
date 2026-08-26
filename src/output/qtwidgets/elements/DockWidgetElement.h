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

class DockWidgetElement : public AbstractElement
{
    Q_OBJECT

public:
    DockWidgetElement(const QStyleOptionDockWidget *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~DockWidgetElement() override;

    void update() override;
    void layout() override;
    void draw(QPainter *painter, DrawEnums enums) const override;
    QRectF subElementRect(QStyle::SubElement subElement) const override;

private:
    void updateSubElementList() override;
    const QStyleOptionDockWidget *m_dockWidgetOption = nullptr;
};
