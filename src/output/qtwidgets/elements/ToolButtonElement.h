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

class ToolButtonElement : public AbstractElement, public std::enable_shared_from_this<ToolButtonElement>
{
    Q_OBJECT

public:
    using Ptr = std::shared_ptr<ToolButtonElement>;
    ToolButtonElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ToolButtonElement() override;
    static ToolButtonElement::Ptr create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget);

    void draw(QPainter *painter) const override;

    QRect subControlRect(QStyle::SubControl subControl) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    const QStyleOptionToolButton *m_toolButtonOption = nullptr;
    QIcon m_indicatorIcon;

    void drawText(QPainter *painter) const override;
    void drawIcon(QPainter *painter) const override;
    void drawIndicator(QPainter *painter) const;

private:
    Union::ElementList m_indicatorElements;
    Union::Properties::StylePropertyGroup *m_indicatorProperties;
};
