// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "AbstractElement.h"
#include "BackgroundDrawing.h"
#include "ElementCache.h"
#include <QIcon>
#include <QObject>
#include <QStyleOption>

class UnionStyle;
class TabElement;

namespace ElementCache
{
namespace detail
{
/* clang-format off */
        template<> struct EnumToType<QStyle::SubElement, QStyle::SE_TabBarTabText> : public TypeHelper<TabElement, QStyleOptionTab>{};
/* clang-format on */
}
}

class TabElement : public AbstractElement
{
    Q_OBJECT

public:
    TabElement(const QStyleOptionTab *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~TabElement() override;

    void update() override;
    void layout() override;
    void draw(QPainter *painter, DrawEnums enums) const override;

    QRectF subElementRect(QStyle::SubElement element) const override;
    QSizeF contentsSize(const QSizeF &contentsSizeFromStyle) const override;

    bool isVertical() const;
    int hSpace() const;
    int vSpace() const;

private:
    QVariantMap elementAttributes() const override;
    QStringList elementHints() const override;
    void updateSubElementList() override;
    const QStyleOptionTab *m_tabOption = nullptr;
    bool m_isVertical;
    bool m_isClosable;
    void tabLayout(QRectF *textRect, QRectF *iconRect) const;
};
