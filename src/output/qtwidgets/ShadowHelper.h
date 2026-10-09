/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 * SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "TileSet.h"

#include "properties/CornersPropertyGroup.h"
#include <QColor>
#include <QMap>
#include <QMargins>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QWindow>

// Adjust the shadow so that its slightly under the widget to
// avoid any gaps. Take this into account when drawing the shadow.
constexpr int SHADOW_ADJUSTMENT = 2;

struct Shadow {
    Shadow() = default;

    Shadow(const QPoint &offset, qreal blur, qreal spread, const QColor &color)
        : offset(offset)
        , blur(blur)
        , spread(spread)
        , color(color)
    {
    }

    QPoint offset;
    qreal blur;
    qreal spread;
    QColor color;
    Union::Properties::CornersPropertyGroup::CornerRadii cornerRadius;
};

class ShadowHelper : public QObject
{
    Q_OBJECT

public:
    explicit ShadowHelper();

    ~ShadowHelper() override;

    static Shadow lookupShadowProperties();

    void reset();

    bool registerWidget(QWidget *, bool force = false);

    void unregisterWidget(QWidget *);

    bool eventFilter(QObject *, QEvent *) override;

    TileSet shadowTiles();

protected Q_SLOTS:

    void widgetDeleted(QObject *);

    void windowDeleted(QObject *);

protected:
    bool acceptWidget(QWidget *) const;

    void installShadows(QWidget *);

    void uninstallShadows(QWidget *);

    QMargins shadowMargins(QWidget *) const;

private:
    //* registered widgets
    QSet<QWidget *> m_widgets;

    //* tileset
    TileSet m_shadowTiles;
};
