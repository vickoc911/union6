// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class FocusElement : public AbstractElement
{
    Q_OBJECT

public:
    FocusElement(const QStyleOptionFocusRect *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~FocusElement() override;

    void update() override;
    void layout() override;

    QStringList elementHints() const override;

private:
    const QStyleOptionFocusRect *m_focusOption = nullptr;
};
