// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include <QIcon>
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class ButtonElement : public AbstractElement
{
    Q_OBJECT

public:
    ButtonElement(const QStyleOptionButton *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ButtonElement() override;

    void update() override;

    void updateSubElementList() override;
    QRectF subElementRect(QStyle::SubElement element) const override;
    QSizeF contentsSize(const QSizeF &contentsSizeFromStyle) const override;

    QStringList elementHints() const override;
    Union::Element::States elementStates() const override;

    const QStyleOptionButton *m_buttonOption = nullptr;
};
