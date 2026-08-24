// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include "BackgroundDrawing.h"
#include <QObject>
#include <QStyleOption>
#include <qstyleoption.h>

class UnionStyle;

class SizeGripElement : public AbstractElement
{
    Q_OBJECT

public:
    SizeGripElement(const QStyleOptionSizeGrip *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~SizeGripElement() override;

    void update() override;
    void drawBackground(QPainter *painter) const override;

    void layout() override;

private:
    const QStyleOptionSizeGrip *m_sizeGripOption = nullptr;
};
