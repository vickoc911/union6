// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include <QObject>
#include <QStyleOption>
#include <qstyleoption.h>

class UnionStyle;

class RubberBandElement : public AbstractElement
{
    Q_OBJECT

public:
    RubberBandElement(const QStyleOptionRubberBand *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~RubberBandElement() override;

    void update() override;
    void layout() override;
    void draw(QPainter *painter, DrawEnums enums) const override;

private:
    const QStyleOptionRubberBand *m_rubberBandOption = nullptr;
};
