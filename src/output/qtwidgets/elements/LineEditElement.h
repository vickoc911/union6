// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class LineEditElement : public AbstractElement
{
    Q_OBJECT

public:
    LineEditElement(const QStyleOptionFrame *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~LineEditElement() override;

    void update() override;
    void draw(QPainter *painter, DrawEnums enums) const override;

    QSizeF iconSize() const override;
    QMarginsF iconPadding() const;
    QRectF subElementRect(QStyle::SubElement element) const override;

private:
    void updateSubElementList() override;
    QStringList elementHints() const override;
    const QStyleOptionFrame *m_frameOption = nullptr;
};
