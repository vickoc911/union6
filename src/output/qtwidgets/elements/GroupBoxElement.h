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

class GroupBoxElement : public AbstractElement
{
    Q_OBJECT

public:
    GroupBoxElement(const QStyleOptionGroupBox *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~GroupBoxElement() override;

    void update() override;
    void layout() override;
    QRectF subControlRect(QStyle::SubControl subControl) const override;

    void drawText(QPainter *painter) const override;
    void drawIcon(QPainter *painter) const override;

    QVariantMap elementAttributes() const override;
    QStringList elementHints() const override;

private:
    const QStyleOptionGroupBox *m_groupBoxOption = nullptr;
    bool m_isCheckable;
};
