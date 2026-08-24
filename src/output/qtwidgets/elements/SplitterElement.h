// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include "BackgroundDrawing.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class SplitterElement : public AbstractElement
{
    Q_OBJECT

public:
    SplitterElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~SplitterElement() override;

    void update() override;
    void drawBackground(QPainter *painter) const override;

    void layout() override;

private:
    const QStyleOption *m_splitterOption = nullptr;
};
