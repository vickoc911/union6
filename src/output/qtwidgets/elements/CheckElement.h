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

class CheckElement : public AbstractElement
{
    Q_OBJECT

public:
    enum class Type {
        CheckBox,
        RadioButton
    };
    Q_ENUM(Type)

    CheckElement(Type type, const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~CheckElement() override;

    void draw(QPainter *painter) const override;

    QRect subElementRect(QStyle::SubElement element) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    const QStyleOptionButton *m_buttonOption = nullptr;

    void updateSubElementList() override;
    void drawIndicator(QPainter *painter) const override;

private:
    Type m_type;
};
