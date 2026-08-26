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

class HeaderElement;

namespace ElementCache
{
namespace detail
{
/* clang-format off */
    template<> struct EnumToType<QStyle::SubElement, QStyle::SE_HeaderArrow> : public TypeHelper<HeaderElement, QStyleOptionHeader>{};
    template<> struct EnumToType<QStyle::SubElement, QStyle::SE_HeaderLabel> : public TypeHelper<HeaderElement, QStyleOptionHeader>{};
/* clang-format on */
}
}

class HeaderElement : public AbstractElement
{
    Q_OBJECT

public:
    HeaderElement(const QStyleOptionHeader *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~HeaderElement() override;

    void update() override;
    void layout() override;
    void draw(QPainter *painter, DrawEnums enums) const override;

    QRectF subElementRect(QStyle::SubElement element) const override;
    QSizeF contentsSize(const QSizeF &contentsSizeFromStyle) const override;

private:
    QStringList elementHints() const override;
    void updateSubElementList() override;
    const QStyleOptionHeader *m_headerOption = nullptr;
    QIcon sortIndicator();
    bool m_isHorizontal;
};
