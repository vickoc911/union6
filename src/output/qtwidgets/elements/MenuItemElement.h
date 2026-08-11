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

class MenuItemElement : public AbstractElement, public std::enable_shared_from_this<MenuItemElement>
{
    Q_OBJECT

public:
    using Ptr = std::shared_ptr<MenuItemElement>;
    MenuItemElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~MenuItemElement() override;
    static MenuItemElement::Ptr create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget);

    void draw(QPainter *painter) const override;

    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    const QStyleOptionMenuItem *m_menuItemOption = nullptr;

    void drawBg(QPainter *painter) const override;
    void drawText(QPainter *painter) const override;
    void drawIndicator(QPainter *painter) const override;
    void layout() override;

private:
    Union::ElementList m_indicatorElementList;
    Union::Properties::StylePropertyGroup *m_indicatorProperties;
    bool m_isSeparator;
    bool m_hasSubMenu;
    bool m_hasCheckBox;
    bool m_hasRadioButton;
    QString m_shortcutText;
};
