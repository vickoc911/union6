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

class ProgressBarElement : public AbstractElement, public std::enable_shared_from_this<ProgressBarElement>
{
    Q_OBJECT

public:
    using Ptr = std::shared_ptr<ProgressBarElement>;
    ProgressBarElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~ProgressBarElement() override;
    static ProgressBarElement::Ptr create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget);

    void draw(QPainter *painter) const override;

    void updateSubElementList() override;
    void layout() override;
    QRect subElementRect(QStyle::SubElement element) const override;
    QSize contentsSize(const QSize &contentsSizeFromStyle) const override;

    const QStyleOptionProgressBar *m_progressBarOption = nullptr;

    void drawGroove(QPainter *painter) const;
    void drawTrack(QPainter *painter) const;
};
