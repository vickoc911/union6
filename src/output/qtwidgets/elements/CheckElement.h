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

class CheckElement;

namespace ElementCache
{
namespace detail
{
/* clang-format off */
    template<> struct EnumToType<QStyle::SubElement, QStyle::SE_RadioButtonContents> : public TypeHelper<CheckElement, QStyleOptionButton>{};
    template<> struct EnumToType<QStyle::SubElement, QStyle::SE_RadioButtonIndicator> : public TypeHelper<CheckElement, QStyleOptionButton>{};
    template<> struct EnumToType<QStyle::SubElement, QStyle::SE_CheckBoxIndicator> : public TypeHelper<CheckElement, QStyleOptionButton>{};
    template<> struct EnumToType<QStyle::SubElement, QStyle::SE_CheckBoxContents> : public TypeHelper<CheckElement, QStyleOptionButton>{};
/* clang-format on */
}
}

class CheckElement : public AbstractElement
{
    Q_OBJECT

public:
    enum class Type {
        CheckBox,
        RadioButton
    };
    Q_ENUM(Type)

    CheckElement(Type type, const QStyleOptionButton *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~CheckElement() override;

    void update() override;
    void draw(QPainter *painter, DrawEnums enums) const override;
    QRectF subElementRect(QStyle::SubElement element) const override;
    QSizeF contentsSize(const QSizeF &contentsSizeFromStyle) const override;

private:
    void updateSubElementList() override;
    void drawIndicator(QPainter *painter) const override;
    QStringList elementHints() const override;
    const QStyleOptionButton *m_buttonOption = nullptr;
    Type m_type;
};
