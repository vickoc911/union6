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

class ButtonElement : public AbstractElement, public std::enable_shared_from_this<ButtonElement>
{
    Q_OBJECT

public:
    using Ptr = std::shared_ptr<ButtonElement>;
    ButtonElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ButtonElement() override;
    static ButtonElement::Ptr create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget);

    void draw(QPainter *painter) const override;

    QRect subElementRect(QStyle::SubElement element) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    const QStyleOptionButton *m_buttonOption = nullptr;
    QIcon m_indicatorIcon;

    void drawIndicator(QPainter *painter) const;

private:
    Union::ElementList m_indicatorElements;
    Union::Properties::StylePropertyGroup *m_indicatorProperties;
};
