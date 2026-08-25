// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class TabBarElement : public AbstractElement
{
    Q_OBJECT

public:
    TabBarElement(const QStyleOptionTabBarBase *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~TabBarElement() override;

    void update() override;
    void drawBackground(QPainter *painter) const override;

    qreal scrollButtonWidth() const;

    QVariantMap elementAttributes() const override;
    QStringList elementHints() const override;

private:
    const QStyleOptionTabBarBase *m_tabBarOption = nullptr;
};
