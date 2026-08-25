// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

// This is a kitchen-sink element class to draw any various indicators

class IndicatorElement : public AbstractElement
{
    Q_OBJECT

public:
    IndicatorElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~IndicatorElement() override;

    void drawArrowLeft(QPainter *painter) const;
    void drawArrowRight(QPainter *painter) const;
    void drawArrowDown(QPainter *painter) const;
    void drawArrowUp(QPainter *painter) const;

private:
    const QStyleOption *m_indicatorOption = nullptr;

    void drawElement(QPainter *painter, const QString &defaultIconName, QStringList targetHierarchy) const;
};
