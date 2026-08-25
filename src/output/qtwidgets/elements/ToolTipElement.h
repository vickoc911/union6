// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class ToolTipElement : public AbstractElement
{
    Q_OBJECT

public:
    ToolTipElement(const QStyleOptionFrame *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ToolTipElement() override;

    void update() override;
    void layout() override;

    QVariantMap elementAttributes() const override;
    QStringList elementHints() const override;

private:
    const QStyleOptionFrame *m_frameOption = nullptr;
};
