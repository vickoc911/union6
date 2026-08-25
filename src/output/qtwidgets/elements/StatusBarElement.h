// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include "BackgroundDrawing.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class StatusBarElement : public AbstractElement
{
    Q_OBJECT

public:
    StatusBarElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~StatusBarElement() override;

    void update() override;
    void drawItem(QPainter *painter) const;

    void layout() override;

private:
    const QStyleOption *m_statusBarOption = nullptr;
    void updateSubElementList() override;
};
