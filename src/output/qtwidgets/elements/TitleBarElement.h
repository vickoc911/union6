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

class TitleBarElement : public AbstractElement, public std::enable_shared_from_this<TitleBarElement>
{
    Q_OBJECT

public:
    using Ptr = std::shared_ptr<TitleBarElement>;
    TitleBarElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~TitleBarElement() override;
    static TitleBarElement::Ptr create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget);

    void draw(QPainter *painter) const override;

    QRect subControlRect(QStyle::SubControl subControl) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    const QStyleOptionTitleBar *m_titleBarOption = nullptr;

    void updateSubElementList() override;
    void layout() override;
};
