// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class TabWidgetElement : public AbstractElement
{
    Q_OBJECT

public:
    TabWidgetElement(const QStyleOptionTabWidgetFrame *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~TabWidgetElement() override;

    void update() override;

private:
    const QStyleOptionTabWidgetFrame *m_tabFrameOption = nullptr;
};
