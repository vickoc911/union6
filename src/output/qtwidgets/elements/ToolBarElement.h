// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include "BackgroundDrawing.h"
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class ToolBarElement : public AbstractElement
{
    Q_OBJECT

public:
    ToolBarElement(const QStyleOptionToolBar *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ToolBarElement() override;

    void update() override;
    void updateSubElementList() override;
    void drawBackground(QPainter *painter) const override;
    void drawFrame(QPainter *painter) const override;

    void drawHandle(QPainter *painter) const;
    void drawSeparator(QPainter *painter) const;
    void layout() override;

private:
    const QStyleOptionToolBar *m_toolBarOption = nullptr;

    Union::ElementList m_handleElementList;
    Union::Properties::StylePropertyGroup *m_handleProperties;
    Union::ElementList m_separatorElementList;
    Union::Properties::StylePropertyGroup *m_separatorProperties;
};
